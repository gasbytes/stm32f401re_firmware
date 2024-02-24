# Use the official Ubuntu image as the base image
FROM ubuntu:latest

# Update package list and install required packages
RUN apt-get update && \
    apt-get install -y gcc-arm-none-eabi make stlink-tools git

# Cleanup to reduce the image size
RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/*
