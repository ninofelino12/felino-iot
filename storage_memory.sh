#!/usr/bin/env bash
# storage_memory.sh — Tampilkan sisa storage (disk) dan pemakaian memory (RAM/swap) pada host Linux.
#
# Usage:
#   ./storage_memory.sh           # ringkasan default
#   ./storage_memory.sh --all     # tampilkan semua filesystem
#   ./storage_memory.sh --watch   # refresh setiap 2 detik (Ctrl+C untuk berhenti)

set -euo pipefail

ALL=false
WATCH=false
for arg in "$@"; do
  case "$arg" in
    --all) ALL=true ;;
    --watch|-w) WATCH=true ;;
    -h|--help)
      sed -n '1,12p' "$0"
      exit 0
      ;;
    *)
      echo "Unknown option: $arg" >&2
      exit 2
      ;;
  esac
done

print_info() {
  # Clear screen only when watching
  if $WATCH; then clear; fi

  echo "================ STORAGE (DISK) ================"
  if $ALL; then
    df -h
  else
    df -h / | awk 'NR==1 || NR==2'
    echo
    echo "Filesystem terbesar (top 5 by size):"
    df -h --output=size,used,avail,pcent,target 2>/dev/null | sort -hr | head -n 6 || true
  fi

  echo
  echo "================ MEMORY (RAM/SWAP) ============="
  free -h

  echo
  echo "Ringkasan:"
  # Available memory in MiB
  avail_kb=$(awk '/^Mem:/ {print $7}' <(free -k))
  total_kb=$(awk '/^Mem:/ {print $2}' <(free -k))
  used_kb=$(awk '/^Mem:/ {print $3}' <(free -k))
  if [ -n "$avail_kb" ] && [ "$total_kb" -gt 0 ]; then
    used_pct=$(( used_kb * 100 / total_kb ))
    avail_pct=$(( avail_kb * 100 / total_kb ))
    echo "  RAM: used ${used_kb} KiB (${used_pct}%), available ${avail_kb} KiB (${avail_pct}%) dari ${total_kb} KiB"
  fi

  # Root partition available
  root_avail=$(df -h / | awk 'NR==2 {print $4}')
  root_pct=$(df -h / | awk 'NR==2 {print $5}')
  echo "  Root (/): sisa ${root_avail} (terpakai ${root_pct})"
}

if $WATCH; then
  while true; do
    print_info
    sleep 2
  done
else
  print_info
fi