#!/usr/bin/env python3
"""
Japanese Character Mapping Generator
Extracts character mapping from font2c output and generates C code
"""

import re
import sys

def extract_char_info(font_file):
    """Extract character information from font2c generated file"""
    characters = []
    
    with open(font_file, 'r') as f:
        content = f.read()
    
    # Find all character comments
    pattern = r'{\s*//\s*misaki_gothic8_([^_\s]+)(?:_0x([0-9a-fA-F]+))?'
    matches = re.findall(pattern, content)
    
    for i, match in enumerate(matches):
        char_name, hex_code = match
        
        if hex_code:
            # Japanese character with Unicode hex code
            unicode_value = int(hex_code, 16)
            characters.append((i, char_name, unicode_value))
        else:
            # ASCII character - map name to ASCII code
            ascii_map = {
                'sp': 0x20,        # space
                'excl': 0x21,      # !
                'quot': 0x22,      # "
                'num': 0x23,       # #
                'dollar': 0x24,    # $
                'percnt': 0x25,    # %
                'amp': 0x26,       # &
                'apos': 0x27,      # '
                'lpar': 0x28,      # (
                'rpar': 0x29,      # )
                'ast': 0x2A,       # *
                'plus': 0x2B,      # +
                'comma': 0x2C,     # ,
                'minus': 0x2D,     # -
                'period': 0x2E,    # .
                'sol': 0x2F,       # /
                '0': 0x30, '1': 0x31, '2': 0x32, '3': 0x33, '4': 0x34,
                '5': 0x35, '6': 0x36, '7': 0x37, '8': 0x38, '9': 0x39,
                'colon': 0x3A,     # :
                'semi': 0x3B,      # ;
                'lt': 0x3C,        # <
                'equals': 0x3D,    # =
                'gt': 0x3E,        # >
                'quest': 0x3F,     # ?
                'commat': 0x40,    # @
                'A': 0x41, 'B': 0x42, 'C': 0x43, 'D': 0x44, 'E': 0x45,
                'F': 0x46, 'G': 0x47, 'H': 0x48, 'I': 0x49, 'J': 0x4A,
                'K': 0x4B, 'L': 0x4C, 'M': 0x4D, 'N': 0x4E, 'O': 0x4F,
                'P': 0x50, 'Q': 0x51, 'R': 0x52, 'S': 0x53, 'T': 0x54,
                'U': 0x55, 'V': 0x56, 'W': 0x57, 'X': 0x58, 'Y': 0x59,
                'Z': 0x5A,
                'lsqb': 0x5B,      # [
                'bsol': 0x5C,      # \
                'rsqb': 0x5D,      # ]
                'circ': 0x5E,      # ^
                'lowbar': 0x5F,    # _
                'grave': 0x60,     # `
                'a': 0x61, 'b': 0x62, 'c': 0x63, 'd': 0x64, 'e': 0x65,
                'f': 0x66, 'g': 0x67, 'h': 0x68, 'i': 0x69, 'j': 0x6A,
                'k': 0x6B, 'l': 0x6C, 'm': 0x6D, 'n': 0x6E, 'o': 0x6F,
                'p': 0x70, 'q': 0x71, 'r': 0x72, 's': 0x73, 't': 0x74,
                'u': 0x75, 'v': 0x76, 'w': 0x77, 'x': 0x78, 'y': 0x79,
                'z': 0x7A,
                'lcub': 0x7B,      # {
                'verbar': 0x7C,    # |
                'rcub': 0x7D,      # }
                'tilde': 0x7E,     # ~
            }
            
            if char_name in ascii_map:
                unicode_value = ascii_map[char_name]
                characters.append((i, char_name, unicode_value))
            else:
                print(f"Warning: Unknown character name: {char_name}")
    
    return characters

def generate_mapping_table(characters):
    """Generate C code for character mapping table"""
    
    # Sort by Unicode value for binary search
    sorted_chars = sorted(characters, key=lambda x: x[2])
    
    lines = []
    lines.append("// Character mapping table - sorted by Unicode value for binary search")
    lines.append("static const char_mapping_t char_map[] = {")
    
    for font_index, char_name, unicode_value in sorted_chars:
        if unicode_value <= 0x7F:
            # ASCII character
            char_display = chr(unicode_value) if 32 <= unicode_value <= 126 else f"\\x{unicode_value:02x}"
            lines.append(f"    {{0x{unicode_value:04X}, {font_index:3d}}},  // {char_display}")
        else:
            # Japanese character
            try:
                char_display = chr(unicode_value)
                lines.append(f"    {{0x{unicode_value:04X}, {font_index:3d}}},  // {char_display}")
            except:
                lines.append(f"    {{0x{unicode_value:04X}, {font_index:3d}}},  // U+{unicode_value:04X}")
    
    lines.append("};")
    lines.append("")
    lines.append(f"#define CHAR_MAP_SIZE {len(characters)}")
    
    return "\n".join(lines)

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 generate_char_mapping.py <font_file.c>")
        sys.exit(1)
    
    font_file = sys.argv[1]
    
    try:
        characters = extract_char_info(font_file)
        print(f"Extracted {len(characters)} characters from {font_file}")
        
        mapping_code = generate_mapping_table(characters)
        print("\n" + mapping_code)
        
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()