#!/usr/bin/env bash
# github_push.sh — Stage, commit, dan push perubahan ke GitHub dari terminal.
#
# Usage:
#   ./github_push.sh "pesan commit"
#   ./github_push.sh            # pakai pesan default dengan timestamp

set -euo pipefail

MSG="${1:-Update: $(date '+%Y-%m-%d %H:%M:%S')}"
BRANCH="$(git rev-parse --abbrev-ref HEAD)"

echo "Branch: $BRANCH"
echo "Commit message: $MSG"

# Pastikan ada perubahan
if git diff --cached --quiet && git diff --quiet && [ -z "$(git status --porcelain)" ]; then
  echo "Tidak ada perubahan untuk di-commit."
  exit 0
fi

git add -A
git commit -m "$MSG"
git push origin "$BRANCH"

echo "Push selesai."