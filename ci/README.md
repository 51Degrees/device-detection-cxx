# API Specific CI/CD Approach
This API does not produce packages so use `tag-repository` approach as described in `common-ci` `README.md`.

Differences:
- There is no `create-packages` pipeline, but `tag-repository` pipeline.
- `deploy` pipeline only push tag and branch to GitHub.
- There is a `smoke` test pipeline that only run a subset of the full test suite. This is meant to be used by developers to get quick feedback on current code changes. The full test suite takes too long to run so it is used in the overnight pipeline and when creating tags.
