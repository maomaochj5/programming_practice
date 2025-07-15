#!/usr/bin/env python3
import struct
import zlib

def crc32(data):
    return zlib.crc32(data) & 0xffffffff

def create_valid_png(filename):
    # PNG signature
    png_signature = b'\x89PNG\r\n\x1a\n'
    
    # IHDR chunk (width=16, height=16, bit_depth=8, color_type=2 RGB, compression=0, filter=0, interlace=0)
    width, height = 16, 16
    ihdr_data = struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0)
    ihdr_crc = crc32(b'IHDR' + ihdr_data)
    ihdr_chunk = struct.pack('>I', len(ihdr_data)) + b'IHDR' + ihdr_data + struct.pack('>I', ihdr_crc)
    
    # Create RGB data for a 16x16 blue square
    # Each pixel is 3 bytes (R, G, B), plus 1 filter byte per row
    row_data = b'\x00' + b'\x00\x00\xff' * width  # Filter byte + 16 blue pixels
    image_data = row_data * height
    
    # Compress the image data
    compressed_data = zlib.compress(image_data)
    
    # IDAT chunk
    idat_crc = crc32(b'IDAT' + compressed_data)
    idat_chunk = struct.pack('>I', len(compressed_data)) + b'IDAT' + compressed_data + struct.pack('>I', idat_crc)
    
    # IEND chunk
    iend_crc = crc32(b'IEND')
    iend_chunk = struct.pack('>I', 0) + b'IEND' + struct.pack('>I', iend_crc)
    
    # Write PNG file
    with open(filename, 'wb') as f:
        f.write(png_signature + ihdr_chunk + idat_chunk + iend_chunk)

# Create all required PNG files
icons = [
    'about.png', 'add_product.png', 'ai_recommendation.png', 'app_icon.png',
    'barcode_scan.png', 'camera.png', 'card.png', 'cash.png', 'connected.png',
    'database.png', 'delete_product.png', 'disconnected.png', 'discount.png',
    'edit_product.png', 'error.png', 'exit.png', 'export_products.png', 'help.png',
    'import_products.png', 'logo.png', 'mobile_pay.png', 'new_sale.png', 'no_image.png',
    'popular.png', 'print_receipt.png', 'product_placeholder.png', 'refresh.png',
    'report.png', 'save_sale.png', 'scan_start.png', 'scan_stop.png', 'search_product.png',
    'settings.png', 'splash.png', 'success.png', 'trending.png', 'warning.png'
]

for icon in icons:
    create_valid_png(icon)
    print(f"Created valid PNG: {icon}")

print("All valid PNG files created successfully!")
