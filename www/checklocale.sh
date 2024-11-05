#!/bin/bash
print_usage() {
	echo "Usage: $0 [option]"
	echo "npm usage: npm run check-locale -- [option]"
	echo "options:"
	echo "  -d|--de check changes in en from last change in de-DE"
	echo "  -j|--ja check changes in en from last change in ja-JP"
	echo "  -k|--ko check changes in en from last change in ko-KR"
	echo "  -p|--pt check changes in en from last change in pt-BR"
	echo "  -z|--zh check changes in en from last change in zh-CN"
	echo "  only one option is allowed"
}
if [[ $# -eq 0 ]]; then
	print_usage
	exit 1
fi
while [[ $# -gt 0 ]]; do
	case $1 in
	-d | --de)
		TARGET_DIR="./src/Locales/de-DE/"
		shift
		;;
	-j | --ja)
		TARGET_DIR="./src/Locales/ja-JP/"
		shift
		;;
	-k | --ko)
		TARGET_DIR="./src/Locales/ko-KR/"
		shift
		;;
	-p | --pt)
		TARGET_DIR="./src/Locales/pt-BR/"
		shift
		;;
	-z | --zh)
		TARGET_DIR="./src/Locales/zh-CN/"
		shift
		;;
	-* | --*)
		echo "Unknown option $1"
		print_usage
		exit 1
		;;
	*)
		echo "Unknown option $1"
		print_usage
		exit 1
		;;
	esac
done

DIFF_PATH="./src/Locales/en/"

# Find the latest commit hash that modified the target Locale directory
LATEST_COMMIT_HASH=$(git log -n 1 --pretty=format:%H -- $TARGET_DIR)

# Check if a commit hash was found
if [ -z "$LATEST_COMMIT_HASH" ]; then
	echo "No commits found for the directory: $TARGET_DIR"
	exit 1
fi

# Run git diff on BASE_LOCALE directory against last change of TARGET_LOCALE directory
echo "Running git diff from commit $LATEST_COMMIT_HASH to HEAD for $DIFF_PATH..."
git diff $LATEST_COMMIT_HASH HEAD -- $DIFF_PATH
