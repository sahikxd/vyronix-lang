name: Bug report
about: Create a report to help us improve
title: '[BUG] '
labels: bug
assignees: ''

body:
  - type: markdown
    attributes:
      value: |
        Thanks for taking the time to fill out this bug report!
  - type: textarea
    id: description
    attributes:
      label: Describe the bug
      description: A clear and concise description of what the bug is.
    validations:
      required: true
  - type: textarea
    id: reproduction
    attributes:
      label: Steps To Reproduce
      description: Steps to reproduce the behavior.
      placeholder: |
        1. Create a file 'test.vx' with code...
        2. Run 'vyronix run test.vx'
        3. See error...
    validations:
      required: true
  - type: textarea
    id: expected
    attributes:
      label: Expected Behavior
      description: A clear and concise description of what you expected to happen.
    validations:
      required: true
  - type: textarea
    id: environment
    attributes:
      label: Environment
      description: OS, Compiler version, etc.
      placeholder: |
        - OS: Windows 10
        - Vyronix Version: 1.0.0
  - type: textarea
    id: logs
    attributes:
      label: Logs or Screenshots
      description: Add any logs or screenshots to help explain your problem.
