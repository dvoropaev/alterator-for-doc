#!/usr/bin/env bash
set -euo pipefail

# === Настройки ===
# Путь-префикс для компонентов внутри монорепы
PREFIX_BASE="${PREFIX_BASE:-subprojects}"
# Необязательный origin для монорепы (например, приватный GitHub-репозиторий)
MONO_ORIGIN="${MONO_ORIGIN:-}"  # пример: git@github.com:your-org/alterator-monorepo.git

# Список upstream-репозиториев Alterator
REPOS=(
  "https://altlinux.space/alterator/alt-components.git"
  "https://altlinux.space/alterator/contributing.git"
  "https://altlinux.space/alterator/alteratorctl.git"
  "https://altlinux.space/alterator/alt-services.git"
  "https://altlinux.space/alterator/alterator-manager.git"
  "https://altlinux.space/alterator/alt-components-base.git"
  "https://altlinux.space/alterator/alterator-service-chrony.git"
  "https://altlinux.space/alterator/diag-domain-client.git"
  "https://altlinux.space/alterator/alterator-backend-component.git"
  "https://altlinux.space/alterator/backlog.git"
  "https://altlinux.space/alterator/alterator-service-samba-ad.git"
  "https://altlinux.space/alterator/alt-control.git"
  "https://altlinux.space/alterator/alterator-backend-control.git"
  "https://altlinux.space/alterator/alterator-module-executor.git"
  "https://altlinux.space/alterator/alterator-test-services.git"
  "https://altlinux.space/alterator/adt.git"
  "https://altlinux.space/alterator/alterator-service-samba-share.git"
  "https://altlinux.space/alterator/alterator-entry.git"
  "https://altlinux.space/alterator/alterator-backend-packages.git"
  "https://altlinux.space/alterator/alterator-backend-component_categories.git"
  "https://altlinux.space/alterator/alterator-backend-batch_components.git"
  "https://altlinux.space/alterator/alterator-backend-batch_component_categories.git"
  "https://altlinux.space/alterator/alterator-backend-edition.git"
  "https://altlinux.space/alterator/alterator-explorer.git"
  "https://altlinux.space/alterator/alterator-module-backend3.git"
  "https://altlinux.space/alterator/alterator-backend-systeminfo.git"
  "https://altlinux.space/alterator/alt-packages.git"
  "https://altlinux.space/alterator/adt-test-tools.git"
  "https://altlinux.space/alterator/alt-systeminfo.git"
  "https://altlinux.space/alterator/alterator-service-nextcloud.git"
  "https://altlinux.space/alterator/alterator-service-vsftpd.git"
  "https://altlinux.space/alterator/remote-polkit-agent.git"
  "https://altlinux.space/alterator/alterator-module-remote.git"
  "https://altlinux.space/alterator/alterator-backend-categories.git"
  "https://altlinux.space/alterator/alterator-interface-diag.git"
  "https://altlinux.space/alterator/alterator-backend-legacy.git"
  "https://altlinux.space/alterator/alterator-interface-application.git"
  "https://altlinux.space/alterator/alterator-interface-service.git"
  "https://altlinux.space/alterator/diag-domain-controller.git"
  "https://altlinux.space/alterator/python3-module-alterator.git"
  "https://altlinux.space/alterator/alterator-how-to-test.git"
)

# --- функции ---
need() { command -v "$1" >/dev/null 2>&1 || { echo "❌ Нужна утилита: $1"; exit 1; }; }

repo_name_from_url() {
  local url="$1"
  basename "$url" .git
}

default_branch_for_url() {
  local url="$1"
  local head
  head="$(git ls-remote --symref "$url" HEAD 2>/dev/null | awk '/^ref:/ {print $2}' | sed 's@refs/heads/@@')"
  if [[ -n "${head:-}" ]]; then
    echo "$head"; return
  fi
  if git ls-remote --heads "$url" main   >/dev/null 2>&1; then echo "main";   return; fi
  if git ls-remote --heads "$url" master >/dev/null 2>&1; then echo "master"; return; fi
  git ls-remote --heads "$url" | awk '{print $2}' | sed 's@refs/heads/@@' | head -n1
}

has_subtree() {
  # Проверяем, что в истории есть метка git-subtree-dir для данного префикса
  local p="$1"
  git log -1 --grep="^git-subtree-dir: $p$" --pretty=%H >/dev/null 2>&1
}

ensure_gitignore_backup() {
  # Гарантируем, что .subtree_backups/ игнорируется
  if [[ ! -f .gitignore ]] || ! grep -qxF ".subtree_backups/" .gitignore; then
    echo ".subtree_backups/" >> .gitignore
    git add .gitignore
    git commit -m "chore: ignore subtree backups" >/dev/null 2>&1 || true
  fi
}

backup_existing_dir() {
  local prefix="$1"
  local ts
  ts="$(date +%Y%m%d%H%M%S)"
  ensure_gitignore_backup
  mkdir -p .subtree_backups
  local target=".subtree_backups/$(basename "$prefix")-$ts"
  echo "🧳 Найден каталог без метаданных subtree: $prefix"
  echo "   Перемещаю в бэкап: $target"
  mv "$prefix" "$target"
}

ensure_remote() {
  local remote="$1"
  local url="$2"
  if git remote get-url "$remote" >/dev/null 2>&1; then
    git remote set-url "$remote" "$url"
  else
    git remote add "$remote" "$url"
  fi
}

add_subtree_fresh() {
  local remote="$1" branch="$2" prefix="$3"
  echo "➕ Добавляю subtree: $remote/$branch → $prefix"
  git fetch "$remote" "$branch"
  git subtree add --prefix="$prefix" "$remote" "$branch" --squash
}

# --- проверки ---
need git

# --- инициализация репозитория ---
if [[ ! -d .git ]]; then
  echo "🧩 Инициализирую новый git-репозиторий..."
  git init
  git checkout -b main
  echo -e "# Alterator monorepo\n\nСобрано с помощью git subtree --squash." > README.md
  git add README.md
  git commit -m "chore: init monorepo"
else
  echo "✅ .git уже существует, пропускаю инициализацию."
fi

# Настроим origin, если задан
if [[ -n "${MONO_ORIGIN}" ]]; then
  if git remote get-url origin >/dev/null 2>&1; then
    git remote set-url origin "$MONO_ORIGIN"
  else
    git remote add origin "$MONO_ORIGIN"
  fi
fi

# --- добавляем/исправляем upstream-репозитории как subtree ---
for url in "${REPOS[@]}"; do
  name="$(repo_name_from_url "$url")"
  remote="$name"
  prefix="${PREFIX_BASE}/${name}"
  branch="$(default_branch_for_url "$url")"

  ensure_remote "$remote" "$url"

  if has_subtree "$prefix"; then
    echo "↪️  [$name] уже добавлен как subtree — пропускаю."
    continue
  fi

  # Если каталога нет — обычное добавление
  if [[ ! -d "$prefix" ]]; then
    add_subtree_fresh "$remote" "$branch" "$prefix"
    continue
  fi

  # Каталог есть, но это не subtree — автоматический бэкап и «чистое» добавление
  backup_existing_dir "$prefix"
  add_subtree_fresh "$remote" "$branch" "$prefix"
done

echo "✅ Все компоненты приведены в состояние subtree в $(pwd)"
echo "💡 Резервные копии прежних каталогов лежат в .subtree_backups/ (игнорируются Git)."
echo "💡 Для обновлений позже запусти: ./pull_updates.sh"
