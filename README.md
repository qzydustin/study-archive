# study-archive

A personal archive for study-period projects and practice code.
Each project is stored as a separate subdirectory under the `main` branch.
For projects imported from external repositories, a dedicated branch with full history is created and pushed to preserve the record.


## Importing a Project (from an Existing Repository)

Use this process when the source repository will be removed or is no longer maintained.
A corresponding branch will be created in this repository to preserve its full commit history.

```bash
# Example setup
PROJECT=<PROJECT_NAME>
UPSTREAM_URL=<UPSTREAM_REPO_URL>
UPSTREAM_BRANCH=main    # or master
BRANCH_NAME="archive/$PROJECT"

# 1. Add and fetch the external repository
git remote remove temp-upstream 2>/dev/null || true
git remote add temp-upstream "$UPSTREAM_URL"
git fetch temp-upstream

# 2. Create a clean branch containing only that repository’s history
git checkout -B "$BRANCH_NAME" "temp-upstream/$UPSTREAM_BRANCH"

# (Optional) After step 2, you may modify the "$BRANCH_NAME" branch before pushing/import:
# e.g., prune files, rename directories, or rewrite history as needed.

# 3. Push the branch to origin to preserve its history
git push -u origin "$BRANCH_NAME"

# 4. Return to main and import the project as a subdirectory
git checkout main
git subtree add --prefix="$PROJECT" "$BRANCH_NAME" \
  -m "Import $PROJECT into subdir"
git push origin main

# 5. Clean up the temporary remote
git remote remove temp-upstream
```

## Result
- The directory `<PROJECT>/` is added under `main`.
- The branch `archive/<PROJECT>` stores the complete historical record.
- The original repository can safely be deleted.


## License

MIT License. If a project includes its own license, retain it within that project’s subdirectory.