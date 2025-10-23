#!/usr/bin/env python3
"""
PlatformIO Pre-Build Script
Memastikan partitions.csv ada dan valid sebelum build
"""

Import("env")
import os
import sys

def check_partition_file(*args, **kwargs):
    """Check if partitions.csv exists and is valid"""
    
    project_dir = env.get("PROJECT_DIR")
    partition_file = os.path.join(project_dir, "partitions.csv")
    
    print("=" * 60)
    print("Pre-Build Check: Partition Table")
    print("=" * 60)
    
    # Check if file exists
    if not os.path.exists(partition_file):
        print(f"❌ ERROR: partitions.csv not found!")
        print(f"Expected location: {partition_file}")
        print(f"Current directory: {os.getcwd()}")
        print(f"Project directory: {project_dir}")
        sys.exit(1)
    
    print(f"✅ Found: {partition_file}")
    
    # Validate content
    try:
        with open(partition_file, 'r') as f:
            lines = f.readlines()
            
        # Remove comments and empty lines
        valid_lines = [
            line.strip() 
            for line in lines 
            if line.strip() and not line.strip().startswith('#')
        ]
        
        # Check for required partitions
        required_partitions = ['nvs', 'otadata', 'factory', 'ota_0', 'ota_1']
        found_partitions = []
        
        for line in valid_lines:
            parts = [p.strip() for p in line.split(',')]
            if len(parts) > 0:
                partition_name = parts[0]
                found_partitions.append(partition_name)
        
        print(f"📊 Partitions found: {', '.join(found_partitions)}")
        
        missing = [p for p in required_partitions if p not in found_partitions]
        if missing:
            print(f"⚠️  WARNING: Missing partitions: {', '.join(missing)}")
        else:
            print("✅ All required partitions present")
        
        # Calculate total size
        total_size = 0
        for line in valid_lines:
            parts = [p.strip() for p in line.split(',')]
            if len(parts) >= 5:
                size_str = parts[4]
                if size_str.startswith('0x'):
                    size = int(size_str, 16)
                    total_size += size
        
        max_size = 4 * 1024 * 1024  # 4MB
        usage_percent = (total_size / max_size) * 100
        
        print(f"💾 Total size: {total_size:,} bytes ({usage_percent:.1f}% of 4MB)")
        
        if total_size > max_size:
            print(f"❌ ERROR: Partition table exceeds 4MB flash size!")
            sys.exit(1)
        
    except Exception as e:
        print(f"⚠️  Warning: Could not validate partition content: {e}")
    
    print("=" * 60)
    print("")

# Register the callback
env.AddPreAction("buildprog", check_partition_file)
