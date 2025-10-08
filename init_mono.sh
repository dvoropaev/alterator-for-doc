#!/usr/bin/env bash
set -euo pipefail

# === Настройки ===
# Путь-префикс для компонентов внутри монорепы
PREFIX_BASE="${PREFIX_BASE:-components}"
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
  if git ls-remote --heads "$url" main >/dev/null 2>&1; then echo "main"; return; fi
  if git ls-remote --heads "$url" master >/dev/null 2>&1; then echo "master"; return; fi
  git ls-remote --heads "$url" | awk '{print $2}' | sed 's@refs/heads/@@' | head -n1
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

# --- добавляем upstream-репозитории как subtree ---
for url in "${REPOS[@]}"; do
  name="$(repo_name_from_url "$url")"
  remote="$name"
  prefix="${PREFIX_BASE}/${name}"
  branch="$(default_branch_for_url "$url")"

  # добавляем/обновляем remote
  if git remote get-url "$remote" >/dev/null 2>&1; then
    git remote set-url "$remote" "$url"
  else
    git remote add "$remote" "$url"
  fi

  # если каталог уже есть — пропускаем
  if [[ -d "$prefix" ]]; then
    echo "↪️  [$name] уже добавлен — пропускаю."
    continue
  fi

  echo "➕ Добавляю $name → $prefix (ветка: $branch)"
  git fetch "$remote" "$branch"
  git subtree add --prefix="$prefix" "$remote" "$branch" --squash
done

echo "✅ Все компоненты добавлены в $(pwd)"
echo "💡 Для обновлений позже запусти: ./pull_updates.sh"
