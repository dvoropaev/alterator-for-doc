#!/usr/bin/env bash
set -euo pipefail

PREFIX_BASE="${PREFIX_BASE:-subprojects}"

need() { command -v "$1" >/dev/null 2>&1 || { echo "❌ Нужна утилита: $1"; exit 1; }; }
need git

mapfile -t REMOTES < <(git remote)

if [[ ${#REMOTES[@]} -eq 0 ]]; then
  echo "⚠️  Нет remotes — запусти ./init_mono.sh сначала."
  exit 1
fi

default_branch_for_remote() {
  local remote="$1"
  local head
  head="$(git ls-remote --symref "$remote" HEAD 2>/dev/null | awk '/^ref:/ {print $2}' | sed 's@refs/heads/@@')"
  if [[ -n "${head:-}" ]]; then
    echo "$head"; return
  fi
  if git ls-remote --heads "$remote" main >/dev/null 2>&1; then echo "main"; return; fi
  if git ls-remote --heads "$remote" master >/dev/null 2>&1; then echo "master"; return; fi
  git ls-remote --heads "$remote" | awk '{print $2}' | sed 's@refs/heads/@@' | head -n1
}

SELECTED=("$@")

should_process() {
  local r="$1"
  if [[ ${#SELECTED[@]} -eq 0 ]]; then return 0; fi
  for s in "${SELECTED[@]}"; do
    [[ "$s" == "$r" ]] && return 0
  done
  return 1
}

for remote in "${REMOTES[@]}"; do
  [[ "$remote" == "origin" ]] && continue
  should_process "$remote" || continue

  prefix="${PREFIX_BASE}/${remote}"
  [[ -d "$prefix" ]] || { echo "⚠️  Каталог $prefix отсутствует — пропускаю."; continue; }

  branch="$(default_branch_for_remote "$remote")"
  echo "🔄 Обновляю $remote (ветка: $branch)"
  git fetch "$remote" "$branch"
  git subtree pull --prefix="$prefix" "$remote" "$branch" --squash
done

echo "✅ Все обновления подтянуты."
