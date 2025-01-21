#!/usr/bin/env python3
import os
import json
import hashlib
import tkinter as tk
from tkinter import filedialog, messagebox
import datetime

CHUNK_SIZE = 4096
CACHE_FILE = ".dyw_cache.json"

# A small dictionary for recognized system file hashes => reference
KNOWN_SYSTEM_FILES = {
    # "dummy_hash_value_for_system_dll": "msvcrt@v6.0.9200.16496"
}

def sha256_hash(data: bytes) -> str:
    hasher = hashlib.sha256()
    hasher.update(data)
    return hasher.hexdigest()

def file_sha256(file_path: str) -> str:
    hasher = hashlib.sha256()
    with open(file_path, "rb") as f:
        while True:
            data = f.read(CHUNK_SIZE)
            if not data:
                break
            hasher.update(data)
    return hasher.hexdigest()

def ensure_chunk_store_exists():
    if not os.path.exists("chunk_store"):
        os.makedirs("chunk_store")

def get_chunk_path(hash_str):
    return os.path.join("chunk_store", hash_str + ".bin")

def chunk_exists(hash_str) -> bool:
    return os.path.isfile(get_chunk_path(hash_str))

def store_chunk(hash_str: str, data: bytes):
    path = get_chunk_path(hash_str)
    if not os.path.exists(path):
        with open(path, "wb") as f:
            f.write(data)

def load_cache() -> dict:
    if os.path.isfile(CACHE_FILE):
        with open(CACHE_FILE, "r", encoding="utf-8") as cf:
            return json.load(cf)
    else:
        return {}

def save_cache(cache_data: dict):
    with open(CACHE_FILE, "w", encoding="utf-8") as cf:
        json.dump(cache_data, cf, indent=2)

def snapshot_directory(directory: str, snapshot_file: str, text_logger=None):
    """
    Create/update a snapshot of 'directory' into 'snapshot_file'.
    This includes incremental logic and known file references.
    """
    ensure_chunk_store_exists()

    # Load or init cache for incremental approach
    cache_data = load_cache()
    if "files" not in cache_data:
        cache_data["files"] = {}

    manifest = {
        "snapshot_version": "1.1-gui",
        "root_directory": os.path.abspath(directory),
        "timestamp": datetime.datetime.now().isoformat(),
        "files": [],
        "top_level_hash": ""
    }

    changed_files_count = 0
    total_files_count = 0

    for root, dirs, files in os.walk(directory):
        for fname in files:
            total_files_count += 1
            fpath = os.path.join(root, fname)

            # Skip if it's the snapshot file or the cache file
            if os.path.abspath(fpath) == os.path.abspath(snapshot_file):
                continue
            if os.path.abspath(fpath) == os.path.abspath(CACHE_FILE):
                continue

            rel_path = os.path.relpath(fpath, start=directory)
            stat = os.stat(fpath)
            modified_time = stat.st_mtime

            in_cache = cache_data["files"].get(rel_path)
            file_hash = None
            has_changed = True

            if in_cache:
                # If mod time didn't change, assume no change for speed
                if abs(in_cache["modified_time"] - modified_time) < 1:
                    has_changed = False
                    file_hash = in_cache["file_hash"]
                else:
                    # Re-hash to confirm
                    file_hash = file_sha256(fpath)
                    if file_hash == in_cache["file_hash"]:
                        has_changed = False
                    else:
                        has_changed = True
            else:
                # Not in cache => definitely changed
                file_hash = file_sha256(fpath)

            if has_changed:
                changed_files_count += 1
                if not file_hash:
                    file_hash = file_sha256(fpath)

                # Known system file?
                if file_hash in KNOWN_SYSTEM_FILES:
                    manifest["files"].append({
                        "path": rel_path,
                        "method": "known_file_ref",
                        "known_ref": KNOWN_SYSTEM_FILES[file_hash],
                        "file_hash": file_hash
                    })
                else:
                    # Chunk-based
                    chunk_hashes = []
                    with open(fpath, "rb") as rf:
                        while True:
                            data = rf.read(CHUNK_SIZE)
                            if not data:
                                break
                            chash = sha256_hash(data)
                            if not chunk_exists(chash):
                                store_chunk(chash, data)
                            chunk_hashes.append(chash)

                    manifest["files"].append({
                        "path": rel_path,
                        "method": "chunked",
                        "chunk_hashes": chunk_hashes,
                        "file_hash": file_hash,
                    })

                cache_data["files"][rel_path] = {
                    "file_hash": file_hash,
                    "modified_time": modified_time,
                    "method": manifest["files"][-1]["method"],
                    "chunk_hashes": manifest["files"][-1].get("chunk_hashes", []),
                    "known_ref": manifest["files"][-1].get("known_ref")
                }
            else:
                old_method = in_cache.get("method")
                old_chunk_hashes = in_cache.get("chunk_hashes", [])
                old_known_ref = in_cache.get("known_ref")

                if old_method == "known_file_ref":
                    manifest["files"].append({
                        "path": rel_path,
                        "method": "known_file_ref",
                        "known_ref": old_known_ref,
                        "file_hash": in_cache["file_hash"],
                    })
                elif old_method == "chunked":
                    manifest["files"].append({
                        "path": rel_path,
                        "method": "chunked",
                        "chunk_hashes": old_chunk_hashes,
                        "file_hash": in_cache["file_hash"]
                    })
                else:
                    # Fallback for any edge case
                    manifest["files"].append({
                        "path": rel_path,
                        "method": "chunked",
                        "chunk_hashes": [],
                        "file_hash": in_cache["file_hash"]
                    })

    # Write initial snapshot
    with open(snapshot_file, "w", encoding="utf-8") as sf:
        json.dump(manifest, sf, indent=2)

    # Compute top-level hash
    with open(snapshot_file, "rb") as sf:
        raw = sf.read()
        top_hash = sha256_hash(raw)

    manifest["top_level_hash"] = top_hash
    with open(snapshot_file, "w", encoding="utf-8") as sf:
        json.dump(manifest, sf, indent=2)

    # Save cache
    save_cache(cache_data)

    if text_logger:
        text_logger(f"Snapshot created: {snapshot_file}")
        text_logger(f"Total files scanned: {total_files_count}, changed: {changed_files_count}")
        text_logger(f"Chunk store: {os.path.abspath('chunk_store')}")
    else:
        print(f"Snapshot created: {snapshot_file}")
        print(f"Total files scanned: {total_files_count}, changed: {changed_files_count}")
        print("Chunk store:", os.path.abspath("chunk_store"))

def restore_snapshot(snapshot_file: str, restore_dir: str, text_logger=None):
    with open(snapshot_file, "r", encoding="utf-8") as sf:
        manifest = json.load(sf)

    # Verify snapshot tampering
    stored_tlh = manifest.get("top_level_hash", "")
    manifest["top_level_hash"] = ""
    raw_manifest = json.dumps(manifest, indent=2).encode("utf-8")
    current_tlh = sha256_hash(raw_manifest)
    if stored_tlh != current_tlh and text_logger:
        text_logger("WARNING: Snapshot might be tampered. Hash mismatch.")
        text_logger(f"Expected: {stored_tlh}, got: {current_tlh}")

    files = manifest["files"]
    os.makedirs(restore_dir, exist_ok=True)

    for fobj in files:
        rel_path = fobj["path"]
        method = fobj.get("method", "chunked")
        file_hash = fobj.get("file_hash", "")
        target_path = os.path.join(restore_dir, rel_path)
        os.makedirs(os.path.dirname(target_path), exist_ok=True)

        if method == "known_file_ref":
            # In a real system, you might retrieve from system_bases or local OS
            # Here we just create an empty file
            with open(target_path, "wb") as out_f:
                out_f.write(b"")
            if text_logger:
                text_logger(f"Restored known-file-ref {rel_path} as empty placeholder.")
        elif method == "chunked":
            chunk_hashes = fobj.get("chunk_hashes", [])
            with open(target_path, "wb") as out_f:
                for chash in chunk_hashes:
                    cpath = os.path.join("chunk_store", chash + ".bin")
                    if not os.path.isfile(cpath):
                        if text_logger:
                            text_logger(f"ERROR: Missing chunk {chash} for {rel_path}.")
                        continue
                    with open(cpath, "rb") as cf:
                        out_f.write(cf.read())
            # Verify
            restored_hash = file_sha256(target_path)
            if restored_hash != file_hash:
                if text_logger:
                    text_logger(f"WARNING: Hash mismatch for {rel_path}.")
                    text_logger(f"Expected {file_hash}, got {restored_hash}")
        else:
            if text_logger:
                text_logger(f"Unknown method {method} for file {rel_path}; skipping.")

    if text_logger:
        text_logger(f"Restore complete. Output at: {os.path.abspath(restore_dir)}")

# ------------------ TKINTER GUI ------------------

class DYWGui(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("DYW Snapshot Tool")
        self.geometry("520x350")
        self.resizable(False, False)

        # Notebook or just frames? Let's do a simple tab-like approach
        self.notebook = tk.Frame(self)
        self.notebook.pack(padx=10, pady=10, fill="both", expand=True)

        # We'll create 2 frames: SnapshotFrame & RestoreFrame
        self.snapshot_frame = tk.LabelFrame(self.notebook, text="Snapshot")
        self.snapshot_frame.grid(row=0, column=0, sticky="nsew", padx=5, pady=5)
        self.restore_frame = tk.LabelFrame(self.notebook, text="Restore")
        self.restore_frame.grid(row=1, column=0, sticky="nsew", padx=5, pady=5)

        self._create_snapshot_ui()
        self._create_restore_ui()

        # Text box for logging
        self.log_box = tk.Text(self.notebook, height=8, width=60, wrap="word")
        self.log_box.grid(row=2, column=0, sticky="nsew", padx=5, pady=5)

    def _create_snapshot_ui(self):
        # Directory to snapshot
        tk.Label(self.snapshot_frame, text="Directory to snapshot:").grid(row=0, column=0, sticky="w", padx=5, pady=3)
        self.snap_dir_var = tk.StringVar()
        tk.Entry(self.snapshot_frame, textvariable=self.snap_dir_var, width=40).grid(row=0, column=1, padx=5, pady=3)
        tk.Button(self.snapshot_frame, text="Browse...", command=self.browse_snap_dir).grid(row=0, column=2, padx=5, pady=3)

        # Output snapshot
        tk.Label(self.snapshot_frame, text="Snapshot JSON output:").grid(row=1, column=0, sticky="w", padx=5, pady=3)
        self.snap_out_var = tk.StringVar()
        tk.Entry(self.snapshot_frame, textvariable=self.snap_out_var, width=40).grid(row=1, column=1, padx=5, pady=3)
        tk.Button(self.snapshot_frame, text="Browse...", command=self.browse_snap_file).grid(row=1, column=2, padx=5, pady=3)

        # Snapshot button
        tk.Button(self.snapshot_frame, text="Create Snapshot", command=self.do_snapshot).grid(row=2, column=1, pady=5)

    def _create_restore_ui(self):
        # Snapshot file
        tk.Label(self.restore_frame, text="Snapshot JSON:").grid(row=0, column=0, sticky="w", padx=5, pady=3)
        self.restore_snap_var = tk.StringVar()
        tk.Entry(self.restore_frame, textvariable=self.restore_snap_var, width=40).grid(row=0, column=1, padx=5, pady=3)
        tk.Button(self.restore_frame, text="Browse...", command=self.browse_restore_snap).grid(row=0, column=2, padx=5, pady=3)

        # Restore directory
        tk.Label(self.restore_frame, text="Restore directory:").grid(row=1, column=0, sticky="w", padx=5, pady=3)
        self.restore_dir_var = tk.StringVar()
        tk.Entry(self.restore_frame, textvariable=self.restore_dir_var, width=40).grid(row=1, column=1, padx=5, pady=3)
        tk.Button(self.restore_frame, text="Browse...", command=self.browse_restore_dir).grid(row=1, column=2, padx=5, pady=3)

        # Restore button
        tk.Button(self.restore_frame, text="Restore Snapshot", command=self.do_restore).grid(row=2, column=1, pady=5)

    def browse_snap_dir(self):
        path = filedialog.askdirectory(title="Select directory to snapshot")
        if path:
            self.snap_dir_var.set(path)

    def browse_snap_file(self):
        # Where to save JSON
        path = filedialog.asksaveasfilename(
            title="Choose snapshot JSON path",
            defaultextension=".json",
            filetypes=[("JSON Files", "*.json"), ("All Files", "*.*")]
        )
        if path:
            self.snap_out_var.set(path)

    def browse_restore_snap(self):
        path = filedialog.askopenfilename(
            title="Select snapshot JSON",
            filetypes=[("JSON Files", "*.json"), ("All Files", "*.*")]
        )
        if path:
            self.restore_snap_var.set(path)

    def browse_restore_dir(self):
        path = filedialog.askdirectory(title="Select restore directory")
        if path:
            self.restore_dir_var.set(path)

    def do_snapshot(self):
        dir_to_snap = self.snap_dir_var.get().strip()
        snap_out = self.snap_out_var.get().strip()
        if not dir_to_snap or not os.path.isdir(dir_to_snap):
            messagebox.showerror("Error", "Invalid directory to snapshot.")
            return
        if not snap_out:
            messagebox.showerror("Error", "Please specify a snapshot output file.")
            return

        try:
            self.log_box.insert(tk.END, f"\n[SNAPSHOT] Processing {dir_to_snap}...\n")
            self.log_box.see(tk.END)
            snapshot_directory(dir_to_snap, snap_out, text_logger=self.gui_logger)
            self.log_box.insert(tk.END, "[SNAPSHOT] Done.\n")
            self.log_box.see(tk.END)
        except Exception as e:
            messagebox.showerror("Snapshot Error", str(e))

    def do_restore(self):
        snap_file = self.restore_snap_var.get().strip()
        restore_folder = self.restore_dir_var.get().strip()
        if not snap_file or not os.path.isfile(snap_file):
            messagebox.showerror("Error", "Invalid snapshot JSON file.")
            return
        if not restore_folder:
            messagebox.showerror("Error", "Please specify a restore directory.")
            return

        try:
            self.log_box.insert(tk.END, f"\n[RESTORE] Restoring from {snap_file}...\n")
            self.log_box.see(tk.END)
            restore_snapshot(snap_file, restore_folder, text_logger=self.gui_logger)
            self.log_box.insert(tk.END, "[RESTORE] Done.\n")
            self.log_box.see(tk.END)
        except Exception as e:
            messagebox.showerror("Restore Error", str(e))

    def gui_logger(self, msg):
        self.log_box.insert(tk.END, msg + "\n")
        self.log_box.see(tk.END)


def main():
    app = DYWGui()
    app.mainloop()

if __name__ == "__main__":
    main()
