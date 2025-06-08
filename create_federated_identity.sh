#!/bin/bash

set -e  # Exit on any error

# Variables
ACR_NAME="todocpp"
SUBSCRIPTION_ID=$(az account show --query id -o tsv)
RESOURCE_GROUP="todo-cpp-project"
AZURE_TENANT_ID=$(az account show --query tenantId -o tsv)
AZURE_AD_APP_NAME="github-acr-oidc"
GITHUB_REPO="ExitoLab/todo_cpp_restful_docker_github_actions_azure"
LOCATION="eastus"

echo "Starting Azure setup for GitHub OIDC..."
echo "Subscription: $SUBSCRIPTION_ID"
echo "Tenant: $AZURE_TENANT_ID"
echo "GitHub Repo: $GITHUB_REPO"

# Create Resource group if it does not exist
echo "Creating resource group..."
az group create --name $RESOURCE_GROUP --location $LOCATION

# Create ACR
echo "Creating Azure Container Registry..."
az acr create --name $ACR_NAME \
  --resource-group $RESOURCE_GROUP \
  --sku Basic \
  --location $LOCATION \
  --admin-enabled true

# Create app registration
echo "Creating Azure AD app registration..."
az ad app create --display-name "$AZURE_AD_APP_NAME"

# Get clientId
APP_ID=$(az ad app list --display-name "$AZURE_AD_APP_NAME" --query "[0].appId" -o tsv)

if [ -z "$APP_ID" ]; then
  echo "Error: Failed to create or retrieve app registration"
  exit 1
fi

echo "App ID: $APP_ID"

# Create service principal from the app ID (skip if already exists)
echo "Creating service principal..."
if ! az ad sp show --id $APP_ID &>/dev/null; then
  az ad sp create --id $APP_ID
  echo "Service principal created."
else
  echo "Service principal already exists, skipping creation."
fi

# Create federated credentials for different scenarios
echo "Creating federated credentials..."

# Main branch
az ad app federated-credential create --id $APP_ID --parameters '{
  "name": "github-oidc-main",
  "issuer": "https://token.actions.githubusercontent.com",
  "subject": "repo:'"$GITHUB_REPO"':ref:refs/heads/main",
  "description": "OIDC GitHub Main Branch",
  "audiences": ["api://AzureADTokenExchange"]
}'

# Pull requests
az ad app federated-credential create --id $APP_ID --parameters '{
  "name": "github-oidc-pr",
  "issuer": "https://token.actions.githubusercontent.com",
  "subject": "repo:'"$GITHUB_REPO"':pull_request",
  "description": "OIDC GitHub Pull Requests",
  "audiences": ["api://AzureADTokenExchange"]
}'

# Get ACR resource ID
ACR_ID=$(az acr show --name $ACR_NAME --query id -o tsv)

# Assign ACR roles
echo "Assigning ACR permissions..."
az role assignment create --assignee $APP_ID \
  --role "AcrPush" \
  --scope $ACR_ID

az role assignment create --assignee $APP_ID \
  --role "AcrPull" \
  --scope $ACR_ID

# Get ACR login server
ACR_LOGIN_SERVER=$(az acr show --name $ACR_NAME --query loginServer -o tsv)

echo ""
echo "=== Setup Complete! ==="
echo ""
echo "Add these secrets to your GitHub repository:"
echo "AZURE_CLIENT_ID: $APP_ID"
echo "AZURE_TENANT_ID: $AZURE_TENANT_ID"
echo "AZURE_SUBSCRIPTION_ID: $SUBSCRIPTION_ID"
echo "ACR_LOGIN_SERVER: $ACR_LOGIN_SERVER"
echo ""
echo "Your GitHub Actions workflow can now authenticate with Azure using OIDC!"
echo "ACR Name: $ACR_NAME"
echo "Resource Group: $RESOURCE_GROUP"