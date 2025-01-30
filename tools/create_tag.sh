#!/usr/bin/env bash
#
# Usage:
#   ./create_and_push_tag.sh <tag_name> [tag_message]
#
# Examples:
#   ./create_and_push_tag.sh v1.0.0
#   ./create_and_push_tag.sh v1.0.0 "First stable release"


if [ $# -lt 1 ]; then
  echo "Usage: $0 <tag_name> [tag_message]"
  exit 1
fi

TAG_NAME="$1"
TAG_MESSAGE="${2:-"Tag created by script"}"

# --- 3. Create an annotated tag
echo "Creating tag '$TAG_NAME' with message '$TAG_MESSAGE' ..."
git tag -a "$TAG_NAME" -m "$TAG_MESSAGE"

# --- 4. Push the tag to remote
echo "Pushing tag '$TAG_NAME' to origin ..."
git push origin "$TAG_NAME"

echo "Tag '$TAG_NAME' pushed successfully."