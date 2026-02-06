# Use an official Node.js runtime as a parent image
FROM node:18-slim

# Install build dependencies for C++ addon
# python3 and make are required by node-gyp
# g++ is required for compiling the C++ code
RUN apt-get update && apt-get install -y \
    python3 \
    make \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /usr/src/app

# Copy package.json and package-lock.json
COPY package*.json ./

# Install dependencies
RUN npm install

# Copy the rest of the application code
COPY . .

# Build the C++ addon
RUN npm run build

# Expose the port the app runs on
EXPOSE 3000

# Define the command to run the app
CMD [ "npm", "start" ]
