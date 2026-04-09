#!/bin/bash

echo "=================================="
echo "Face Auth Installation Script"
echo "=================================="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run with sudo: sudo ./install.sh"
    exit 1
fi

echo "Installing face authentication system..."

# Create directories
echo "Creating directories..."
mkdir -p /usr/local/bin
mkdir -p /usr/local/share/face-auth/models
mkdir -p /usr/local/share/face-auth/data/users
mkdir -p /lib/security

# Copy binaries
echo "Installing binaries..."
cp build/face-auth-enroll /usr/local/bin/
cp build/face-auth-verify /usr/local/bin/
chmod +x /usr/local/bin/face-auth-enroll
chmod +x /usr/local/bin/face-auth-verify

# Copy PAM module
echo "Installing PAM module..."
cp build/libpam_face_auth.so /lib/security/pam_face_auth.so
chmod 644 /lib/security/pam_face_auth.so

# Copy models
echo "Installing face detection models..."
cp models/* /usr/local/share/face-auth/models/

# Copy user data if exists
if [ -d "build/data/users" ]; then
    echo "Copying enrolled user data..."
    cp -r build/data/users/* /usr/local/share/face-auth/data/users/ 2>/dev/null || true
fi

# Set permissions
echo "Setting permissions..."
chmod 755 /usr/local/share/face-auth
chmod 755 /usr/local/share/face-auth/models
chmod 755 /usr/local/share/face-auth/data
chmod 755 /usr/local/share/face-auth/data/users
chmod 644 /usr/local/share/face-auth/models/*
chmod 600 /usr/local/share/face-auth/data/users/* 2>/dev/null || true

echo ""
echo "✓ Installation complete!"
echo ""
echo "=================================="
echo "IMPORTANT: Before enabling PAM"
echo "=================================="
echo "1. Test the verify program first:"
echo "   sudo /usr/local/bin/face-auth-verify $USER"
echo ""
echo "2. Enroll users (from their account):"
echo "   face-auth-enroll"
echo ""
echo "3. When ready, configure PAM:"
echo "   sudo nano /etc/pam.d/common-auth"
echo "   Add this line AFTER 'pam_unix.so':"
echo "   auth sufficient pam_face_auth.so"
echo ""
echo "WARNING: Keep a terminal open with sudo access"
echo "until you verify face auth works!"
echo "=================================="
