#!/usr/bin/env python3

import sys
import os
import json
import gzip
import argparse
import re
from collections import Counter

def strip_punctuation(text):
    """
    Remove basic punctuation characters using a simple regex.
    You can adjust this to keep certain punctuation if desired.
    """
    # This regex removes anything that is NOT alphanumeric or whitespace
    return re.sub(r'[^a-zA-Z0-9\s]', '', text)

def build_dictionary(text, max_entries=100):
    """
    Build a simple frequency-based dictionary from the input text.
    
    :param text: The raw text (already preprocessed) to analyze
    :param max_entries: Maximum number of dictionary entries to keep
    :return: A dict mapping words -> tokens
    """
    words = text.split()
    freq = Counter(words)
    # Get the top N most common words
    common_words = freq.most_common(max_entries)
    token_map = {}
    for i, (word, _) in enumerate(common_words):
        token = f"__T{i}__"  # Keep the token format short if desired
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
    # Reverse mapping: token -> word
    rev_map = {v: k for k, v in token_map.items()}
    words = encoded_text.split()
    decoded_words = [rev_map.get(word, word) for word in words]
    return ' '.join(decoded_words)

def encode_file(input_file, output_json, max_dict, to_lower, strip_punc_flag, gzip_flag):
    """
    Read a text file, optionally strip punctuation and lower-case it,
    build dictionary, encode the data, and store JSON metadata compactly.
    Optionally gzip the final file.
    """
    # 1. Read the input file
    with open(input_file, 'r', encoding='utf-8') as f:
        original_text = f.read()
    
    # 2. Preprocess text (optional steps)
    processed_text = original_text
    if to_lower:
        processed_text = processed_text.lower()
    if strip_punc_flag:
        processed_text = strip_punctuation(processed_text)
    
    # 3. Build dictionary
    token_map = build_dictionary(processed_text, max_entries=max_dict)
    
    # 4. Encode the text
    encoded_data = encode_text(processed_text, token_map)
    
    # 5. Prepare metadata
    file_info = {
        "file_name": os.path.basename(input_file),
        "original_size": len(original_text.encode('utf-8')),
        "dictionary_size": len(token_map),
        "to_lower": to_lower,
        "strip_punc": strip_punc_flag
    }
    
    # 6. Construct JSON structure (no fancy indentation)
    payload = {
        "metadata": file_info,
        "dictionary": token_map,
        "encoded_data": encoded_data
    }
    
    # Convert to JSON string with minimal separators
    json_bytes = json.dumps(payload, ensure_ascii=False, separators=(',', ':')).encode('utf-8')
    
    # 7. Write to output (optionally gzip)
    if gzip_flag:
        # If user wants GZip, add .gz extension automatically
        if not output_json.endswith('.gz'):
            output_json += '.gz'
        with gzip.open(output_json, 'wb') as gf:
            gf.write(json_bytes)
        print(f"Encoded {input_file} → {output_json} (gzipped)")
    else:
        with open(output_json, 'wb') as f:
            f.write(json_bytes)
        print(f"Encoded {input_file} → {output_json}")
    
    # 8. Print stats
    encoded_size = len(encoded_data.encode('utf-8'))
    dict_size_approx = sum(len(k.encode('utf-8')) + len(v.encode('utf-8')) 
                           for k, v in token_map.items())
    total_stored = len(json_bytes)  # the size of the JSON we wrote to disk
    original_size = file_info["original_size"]
    
    print(f"  Original size: {original_size} bytes")
    print(f"  Encoded data size (in memory): {encoded_size} bytes")
    print(f"  Dictionary size approx (in memory): {dict_size_approx} bytes")
    print(f"  Final on-disk payload: {total_stored} bytes (before any OS-level compression)")

def decode_file(input_json, output_file):
    """
    Read the JSON or gzipped JSON, reconstruct the original text, and save to output_file.
    We assume the user wants to restore the *preprocessed* version (punctuation stripped, etc.)
    exactly as it was after we applied the transformations. 
    """
    # 1. Detect if the input is gzipped
    is_gz = input_json.endswith('.gz')
    
    # 2. Load the JSON payload
    if is_gz:
        with gzip.open(input_json, 'rb') as gf:
            json_bytes = gf.read()
    else:
        with open(input_json, 'rb') as f:
            json_bytes = f.read()
    
    payload = json.loads(json_bytes.decode('utf-8'))
    
    metadata = payload["metadata"]
    token_map = payload["dictionary"]
    encoded_data = payload["encoded_data"]
    
    # 3. Decode the text
    reconstructed_text = decode_text(encoded_data, token_map)
    
    # 4. Write the reconstructed text to output
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(reconstructed_text)
    
    print(f"Decoded {input_json} → {output_file}")
    print(f"  Original file name was: {metadata['file_name']}")
    print(f"  Original size was: {metadata['original_size']} bytes")
    print(f"  Preprocessing used (to_lower={metadata['to_lower']}, strip_punc={metadata['strip_punc']})")

def main():
    parser = argparse.ArgumentParser(description="Refined dictionary-based compression.")
    subparsers = parser.add_subparsers(dest='command')
    
    # Encode sub-command
    encode_parser = subparsers.add_parser('encode', help='Encode a file')
    encode_parser.add_argument('input_file', type=str, help='Path to input text file')
    encode_parser.add_argument('output_json', type=str, help='Path to output JSON file')
    encode_parser.add_argument('--max_dict', type=int, default=100, 
                               help='Maximum dictionary entries (default=100)')
    encode_parser.add_argument('--to_lower', action='store_true', 
                               help='Convert text to lowercase before encoding')
    encode_parser.add_argument('--strip_punc', action='store_true', 
                               help='Strip punctuation before encoding')
    encode_parser.add_argument('--gzip', action='store_true', 
                               help='Gzip the final JSON')
    
    # Decode sub-command
    decode_parser = subparsers.add_parser('decode', help='Decode a file')
    decode_parser.add_argument('input_json', type=str, help='Path to encoded JSON or JSON.gz')
    decode_parser.add_argument('output_file', type=str, help='Path to output reconstructed text file')
    
    args = parser.parse_args()
    
    if args.command == 'encode':
        encode_file(
            input_file=args.input_file,
            output_json=args.output_json,
            max_dict=args.max_dict,
            to_lower=args.to_lower,
            strip_punc_flag=args.strip_punc,
            gzip_flag=args.gzip
        )
    elif args.command == 'decode':
        decode_file(
            input_json=args.input_json,
            output_file=args.output_file
        )
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
