# Todo List C++ RESTFul Docker, GitHub Actions, and AWS ECR

This project demonstrates creating a robust CI/CD pipeline for a C++ Restful Todo List application using Docker, GitHub Actions, and AWS Elastic Container Registry (ECR).

## Table of Contents

- [Project Overview](#project-overview)
- [Technologies Used](#technologies-used)
- [Prerequisites](#prerequisites)
- [Running the Pipeline](#running-the-pipeline)

## Project Overview

This project provides a CI/CD pipeline for a C++ application with the following capabilities:
- **Code Compilation**: Automates the build process with CMake.
- **Dockerization**: Containerizes the application for deployment.
- **Deployment**: Pushes the Docker image to Azure Container Registry (ACR).

## Technologies Used

- **C++**: For the application code.
- **CMake**: To manage the build system.
- **Docker**: For containerization.
- **GitHub Actions**: For CI/CD automation.
- **Azure Container Registry**: To store and manage Docker images.

## Prerequisites

- **Docker**: Installed and configured.
- **GitHub Repository**: For hosting the project code and workflows.

## Running the Pipeline
- Commit and push your code to GitHub.
- Monitor the workflow in the GitHub Actions tab.
- Verify the Docker image in your ACR repository.
