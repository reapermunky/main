#!/usr/bin/env python3

import sys
import os
import json
from collections import Counter

def build_dictionary(text, max_entries=1000):
    """
    Build a simple word-based dictionary from the input text.
    
    :param text: The raw text to analyze
    :param max_entries: Maximum number of dictionary entries to keep
    :return: A dict mapping words -> tokens
    """
    words = text.split()
    freq = Counter(words)
    # Get the top N most common words
    common_words = freq.most_common(max_entries)
    # Create a token mapping
    token_map = {}
    for i, (word, _) in enumerate(common_words):
        # Your token can be any format you want; just ensure uniqueness
        token = f"__T{i}__"
        token_map[word] = token
    return token_map

def encode_text(text, token_map):
    """
    Replace occurrences of common words with tokens.
    
    :param text: Original text
    :param token_map: dict of {word -> token}
    :return: Encoded (compressed) text as a single string
    """
    words = text.split()
    encoded_words = [token_map.get(word, word) for word in words]
    return ' '.join(encoded_words)

def decode_text(encoded_text, token_map):
    """
    Revert tokens back to original words.
    
    :param encoded_text: Tokenized text
    :param token_map: dict of {word -> token}
    :return: Decoded (original) text
    """
    # Reverse the mapping: token -> word
    rev_map = {v: k for k, v in token_map.items()}
    
    words = encoded_text.split()
    decoded_words = [rev_map.get(word, word) for word in words]
    return ' '.join(decoded_words)

def encode_file(input_file, output_json):
    """
    Read a text file, build dictionary, encode the data, and store JSON metadata.
    """
    # 1. Read the input file
    with open(input_file, 'r', encoding='utf-8') as f:
        original_text = f.read()
    
    # 2. Build dictionary
    token_map = build_dictionary(original_text, max_entries=1000)
    
    # 3. Encode the text
    encoded_data = encode_text(original_text, token_map)
    
    # 4. Prepare metadata
    file_info = {
        "file_name": os.path.basename(input_file),
        "original_size": len(original_text.encode('utf-8')),
        "dictionary_size": len(token_map),
    }
    
    # 5. Construct JSON structure
    payload = {
        "metadata": file_info,
        "dictionary": token_map,
        "encoded_data": encoded_data
    }
    
    # 6. Write to output .json
    with open(output_json, 'w', encoding='utf-8') as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)
    
    # Optional: Print some stats
    encoded_size = len(encoded_data.encode('utf-8'))
    dict_size_approx = sum(len(k.encode('utf-8')) + len(v.encode('utf-8')) 
                           for k,v in token_map.items())
    total_stored = encoded_size + dict_size_approx
    original_size = file_info["original_size"]
    
    print(f"Encoded {input_file} → {output_json}")
    print(f"  Original size: {original_size} bytes")
    print(f"  Encoded data size: {encoded_size} bytes")
    print(f"  Dictionary size approx: {dict_size_approx} bytes")
    print(f"  Total stored (encoded + dictionary): {total_stored} bytes")
    
def decode_file(input_json, output_file):
    """
    Read the JSON file, reconstruct the original text, and save to output_file.
    """
    # 1. Read JSON
    with open(input_json, 'r', encoding='utf-8') as f:
        payload = json.load(f)
    
    metadata = payload["metadata"]
    token_map = payload["dictionary"]
    encoded_data = payload["encoded_data"]
    
    # 2. Decode the text
    reconstructed_text = decode_text(encoded_data, token_map)
    
    # 3. Write the reconstructed text to output
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(reconstructed_text)
    
    print(f"Decoded {input_json} → {output_file}")
    print(f"  Original file name was: {metadata['file_name']}")
    print(f"  Original size was: {metadata['original_size']} bytes")

def main():
    if len(sys.argv) < 2:
        print("Usage:")
        print("  python databuild.py encode <input_file> <output_json>")
        print("  python databuild.py decode <input_json> <output_file>")
        sys.exit(1)
    
    command = sys.argv[1].lower()
    
    if command == "encode":
        if len(sys.argv) != 4:
            print("Usage: python databuild.py encode <input_file> <output_json>")
            sys.exit(1)
        input_file = sys.argv[2]
        output_json = sys.argv[3]
        encode_file(input_file, output_json)
    
    elif command == "decode":
        if len(sys.argv) != 4:
            print("Usage: python databuild.py decode <input_json> <output_file>")
            sys.exit(1)
        input_json = sys.argv[2]
        output_file = sys.argv[3]
        decode_file(input_json, output_file)
    
    else:
        print(f"Unknown command: {command}")
        sys.exit(1)

if __name__ == "__main__":
    main()
