#!/usr/bin/env python3
import struct

def create_minimal_png(filename):
    # PNG header
    png_header = b'\x89PNG\r\n\x1a\n'
    
    # IHDR chunk (13 bytes data: width=1, height=1, bit_depth=8, color_type=2, compression=0, filter=0, interlace=0)
    ihdr_data = struct.pack('>IIBBBBB', 1, 1, 8, 2, 0, 0, 0)
    ihdr_crc = 0x7E7B1C13  # Pre-calculated CRC for this IHDR
    ihdr_chunk = struct.pack('>I', 13) + b'IHDR' + ihdr_data + struct.pack('>I', ihdr_crc)
    
    # IDAT chunk (minimal RGB data for 1x1 red pixel)
    idat_data = b'\x08\x1d\x01\x02\x00\xfd\xff\x00\x00\x00\x02\x00\x01'
    idat_crc = 0x90773153  # Pre-calculated CRC
    idat_chunk = struct.pack('>I', len(idat_data)) + b'IDAT' + idat_data + struct.pack('>I', idat_crc)
    
    # IEND chunk
    iend_crc = 0xAE426082  # Pre-calculated CRC for IEND
    iend_chunk = struct.pack('>I', 0) + b'IEND' + struct.pack('>I', iend_crc)
    
    # Write PNG file
    with open(filename, 'wb') as f:
        f.write(png_header + ihdr_chunk + idat_chunk + iend_chunk)

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
    create_minimal_png(icon)
    print(f"Created {icon}")

print("All PNG files created successfully!")
