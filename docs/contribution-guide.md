# How to Contribute

Thank you for taking the time to read this and contributing to the project! 

Third-party contributions help us grow and improve GP2040-CE. We want to make the pull request and contribution process useful and easy for both contributors and maintainers. 

To this end we’ve put together some guidelines for contributors to help your pull request be accepted without major changes.

## Project Overview

GP2040-CE is largely written in C/C++, with a significant portion of the web configurator written in JavaScript within a React framework.

## How do I make a contribution?

Never made an open source contribution before? Wondering how contributions work in GP2040-CE? Here’s a quick rundown!

1. Sign up for a GitHub account.
2. Find an issue you are interested in addressing, or a feature you would like to add.
3. Fork the repository associated with the issue to your GitHub account. This means that you will have a copy of the repository under `your-GitHub-username/GP2040-CE`.
4. Clone the repository to your local machine using `git clone https://github.com/OpenStickCommunity/GP2040-CE`
5. If you’re working on a new feature consider opening an issue to talk with us about the work you’re about to undertake.
6. Create a new branch for your fix using `git checkout -b branch-name-here`.
7. Make the appropriate changes for the issue you are trying to address or the feature that you want to add.
8. Use `git add insert-paths-of-changed-files-here` to add the file contents of the changed files to the “snapshot” git uses to manage the state of the project, also known as the index.
9. Use `git commit -m "Insert a short message of the changes made here"` to store the contents of the index with a descriptive message.
10. Push the changes to your repository on GitHub using `git push origin branch-name-here`.
11. Submit a pull request to [OpenStickCommunity/GP2040-CE](https://github.com/OpenStickCommunity/GP2040-CE).
12. Title the pull request with a short description of the changes made and the issue or bug number associated with your change. For example, you can title an issue like so “Added more log outputting to resolve #4352”.
13. In the description of the pull request explain the changes that you made, any issues you think exist with the pull request you made, and any questions you have for the maintainer. It’s OK if your pull request is not perfect (no pull request is), the reviewer will be able to help you fix any problems and improve it!
14. Wait for the pull request to be reviewed by a maintainer.
15. Make changes to the pull request if the reviewing maintainer recommends them.
16. Celebrate your success after your pull request is merged!

## Coding Conventions

## General Guidelines

- **Before you contribute**: Please make sure your fork is up to date with the upstream `GP2040-CE` repo. This will help minimize CI failures that may not occur for you when compiling locally.
- Separate PRs into logical units. For example, do not submit one PR covering two separate features, instead submit a separate PR for each feature.
- Check for unnecessary whitespace with `git diff --check` before committing.
- Make sure your code change actually compiles.
- Make sure commit messages are understandable on their own. You should put a short description (no more than 70 characters) on the first line, the second line should be empty, and on the 3rd and later lines you should describe your commit in detail, if required.

## Documentation

Documentation is one of the easiest ways to get started contributing to GP2040-CE. Finding places where the documentation is wrong or incomplete and fixing those is easy! We also very badly need someone to edit our documentation, so if you have editing skills but aren’t sure where or how to jump in please [reach out for help](getting-help-support "GP2040-CE | Getting Help")!

### Previewing Documentation

1. Open a terminal in the root of the GP2040-CE project
2. Change the directory to `docs`
3. Run the following commands
  ```
  npm install
  npm start
  ```
4. In a web browser, go to `http://localhost:3000`
