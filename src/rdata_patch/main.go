package main

import (
	"encoding/binary"
	"fmt"
	"os"
	"strings"
)

const (
	IMAGE_SCN_MEM_WRITE = 0x80000000
)

func main() {
	if len(os.Args) != 3 {
		fmt.Fprintf(os.Stderr, "Usage: %s <input_file> <output_file>\n", os.Args[0])
		os.Exit(1)
	}

	inputPath := os.Args[1]
	outputPath := os.Args[2]

	fmt.Printf("Input:  %s\n", inputPath)
	fmt.Printf("Output: %s\n", outputPath)

	if err := makeRdataWritable(inputPath, outputPath); err != nil {
		fmt.Fprintf(os.Stderr, "Error: %v\n", err)
		os.Exit(1)
	}

	fmt.Println("Success: .rdata section is now writable in the output file.")
}

func makeRdataWritable(inputPath, outputPath string) error {
	data, err := os.ReadFile(inputPath)
	if err != nil {
		return fmt.Errorf("read input: %w", err)
	}

	modified := make([]byte, len(data))
	copy(modified, data)

	if len(modified) < 0x40 {
		return fmt.Errorf("file too small for DOS header")
	}

	eLfanew := binary.LittleEndian.Uint32(modified[0x3C:0x40])
	if eLfanew == 0 || eLfanew >= uint32(len(modified)) {
		return fmt.Errorf("invalid e_lfanew offset: %d", eLfanew)
	}

	if uint64(eLfanew)+4 > uint64(len(modified)) {
		return fmt.Errorf("file too small for NT header")
	}

	sig := binary.LittleEndian.Uint32(
		modified[int(eLfanew) : int(eLfanew)+4],
	)
	if sig != 0x00004550 {
		return fmt.Errorf("invalid PE signature")
	}

	offsetNumSections := uint64(eLfanew) + 0x06
	if offsetNumSections+2 > uint64(len(modified)) {
		return fmt.Errorf("file too small for NumberOfSections")
	}

	numSections := binary.LittleEndian.Uint16(
		modified[int(offsetNumSections) : int(offsetNumSections)+2],
	)

	// PE signature: 4 bytes
	// SizeOfOptionalHeader offset in IMAGE_FILE_HEADER: 16 bytes
	offsetSizeOpt := uint64(eLfanew) + 0x14
	if offsetSizeOpt+2 > uint64(len(modified)) {
		return fmt.Errorf("file too small for SizeOfOptionalHeader")
	}

	sizeOptHdr := binary.LittleEndian.Uint16(
		modified[int(offsetSizeOpt) : int(offsetSizeOpt)+2],
	)

	// PE signature (4) + IMAGE_FILE_HEADER (20) = 0x18
	sectionTableOffset := uint64(eLfanew) + 0x18 + uint64(sizeOptHdr)
	if sectionTableOffset > uint64(len(modified)) {
		return fmt.Errorf("file too small for section table")
	}

	found := false

	for i := 0; i < int(numSections); i++ {
		offset := sectionTableOffset + uint64(i)*40
		if offset+40 > uint64(len(modified)) {
			return fmt.Errorf("section header %d out of bounds", i)
		}

		nameBytes := modified[int(offset) : int(offset)+8]

		// PE 节名字段固定为 8 字节，未使用部分通常以 0 填充。
		name := strings.TrimRight(string(nameBytes), "\x00")

		if strings.HasPrefix(name, ".rdata") {
			charOffset := offset + 0x24
			if charOffset+4 > uint64(len(modified)) {
				return fmt.Errorf("characteristics field out of bounds")
			}

			oldChars := binary.LittleEndian.Uint32(
				modified[int(charOffset) : int(charOffset)+4],
			)

			fmt.Printf(
				"Found section '%s' with old characteristics: 0x%08X\n",
				name,
				oldChars,
			)

			newChars := oldChars | IMAGE_SCN_MEM_WRITE

			if newChars == oldChars {
				fmt.Println("Section is already writable; no change made.")
			} else {
				binary.LittleEndian.PutUint32(
					modified[int(charOffset):int(charOffset)+4],
					newChars,
				)

				fmt.Printf(
					"Updated characteristics to: 0x%08X\n",
					newChars,
				)
			}

			found = true
			break
		}
	}

	if !found {
		return fmt.Errorf("no .rdata section found in the PE file")
	}

	if err := os.WriteFile(outputPath, modified, 0644); err != nil {
		return fmt.Errorf("write output: %w", err)
	}

	return nil
}

