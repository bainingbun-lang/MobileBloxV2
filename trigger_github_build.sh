#!/bin/bash
# Trigger GitHub Actions build for MobileBloxV2 ARM64
# Requiere: gh CLI autenticado (gh auth login)

set -euo pipefail

REPO="tu-usuario/MobileBloxV2"  # CAMBIAR A TU REPO
VERSION="${1:-2.0.0}"

echo "🚀 Triggering GitHub Actions build for MobileBloxV2 ARM64"
echo "Repo: $REPO"
echo "Version: $VERSION"
echo ""

# Verificar gh CLI
if ! command -v gh &> /dev/null; then
    echo "❌ gh CLI no instalado. Instala con: pkg install gh"
    exit 1
fi

# Verificar autenticación
if ! gh auth status &> /dev/null; then
    echo "❌ No autenticado. Ejecuta: gh auth login"
    exit 1
fi

# Trigger workflow
echo "📦 Disparando workflow build-arm64.yml..."
gh workflow run build-arm64.yml \
  --repo "$REPO" \
  -f version="$VERSION"

echo ""
echo "✅ Build disparado. Ver progreso en:"
echo "   https://github.com/$REPO/actions"
echo ""
echo "📱 Cuando termine, descarga el APK desde:"
echo "   https://github.com/$REPO/actions/runs/<RUN_ID>"
echo ""
echo "🔗 O crea release automático:"
echo "   gh workflow run build-arm64.yml --repo $REPO -f version=$VERSION"