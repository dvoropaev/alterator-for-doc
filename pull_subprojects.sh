#!/bin/bash
set -euo pipefail

# При ошибке показываем строку
trap 'echo "❌ Ошибка при выполнении скрипта (строка $LINENO)"; exit 1' ERR

REPOS=(
  "https://altlinux.space/sheriffkorov/dbusxml-to-md-xslt-template.git"
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

TARGET_DIR="./subprojects"

echo "🧹 Полностью очищаю $TARGET_DIR ..."
rm -rf "$TARGET_DIR"
mkdir -p "$TARGET_DIR"

echo "⬇️ Клонирую репозитории..."
for repo in "${REPOS[@]}"; do
    name=$(basename "$repo" .git)
    path="$TARGET_DIR/$name"

    echo "➡️  Клонирую $name ..."
    git clone --depth=1 "$repo" "$path"
done

echo "🧨 Удаляю скрытые каталоги (кроме .gear)..."

for repo_dir in "$TARGET_DIR"/*; do
    [ -d "$repo_dir" ] || continue

    while IFS= read -r -d '' dotdir; do
        # Если каталог — именно ".gear", пропускаем
        [[ "$(basename "$dotdir")" == ".gear" ]] && continue

        echo "   Удаляю $dotdir"
        rm -rf "$dotdir"
    done < <(find "$repo_dir" -mindepth 1 -type d -name ".*" -print0)
done

echo "✅ Готово. Все репозитории склонированы в $TARGET_DIR, скрытые каталоги (кроме .gear) удалены."
