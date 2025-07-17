#!/usr/bin/env python3
import json
import sqlite3
import sys

def import_products():
    # Connect to database
    conn = sqlite3.connect('pos_database.db')
    cursor = conn.cursor()
    
    # Create Products table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS Products (
            product_id INTEGER PRIMARY KEY AUTOINCREMENT,
            barcode TEXT UNIQUE NOT NULL,
            name TEXT NOT NULL,
            description TEXT,
            price REAL NOT NULL CHECK(price >= 0),
            stock_quantity INTEGER NOT NULL CHECK(stock_quantity >= 0),
            category TEXT,
            image_path TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    
    # Clear existing data
    cursor.execute('DELETE FROM Products')
    
    # Load JSON data
    with open('Ai_model/products.json', 'r', encoding='utf-8') as f:
        products = json.load(f)
    
    # Create AI ID to DB ID mapping
    ai_to_db_mapping = {}
    
    # Insert products and create mapping
    for product in products:
        ai_id = product['product_id']
        name = product['product_name']
        category = product['category']
        price = product['price']
        description = product['description']
        
        # Insert into database
        cursor.execute('''
            INSERT INTO Products (barcode, name, description, price, stock_quantity, category, image_path)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (ai_id, name, description, price, 100, category, ''))
        
        # Get the auto-generated ID
        db_id = cursor.lastrowid
        ai_to_db_mapping[ai_id] = db_id
        
        print(f"Imported: {name} - AI_ID: {ai_id} -> DB_ID: {db_id}")
    
    # Save the mapping to a JSON file
    with open('Ai_model/ai_to_db_id_mapping.json', 'w', encoding='utf-8') as f:
        json.dump(ai_to_db_mapping, f, ensure_ascii=False, indent=2)
    
    # Also create reverse mapping for database queries
    db_to_ai_mapping = {str(db_id): ai_id for ai_id, db_id in ai_to_db_mapping.items()}
    with open('Ai_model/db_to_ai_id_mapping.json', 'w', encoding='utf-8') as f:
        json.dump(db_to_ai_mapping, f, ensure_ascii=False, indent=2)
    
    conn.commit()
    
    # Verify the import
    cursor.execute('SELECT COUNT(*) FROM Products')
    count = cursor.fetchone()[0]
    print(f"\nSuccessfully imported {count} products to database")
    
    # Show first few entries for verification
    cursor.execute('SELECT product_id, barcode, name, price FROM Products LIMIT 5')
    rows = cursor.fetchall()
    print("\nFirst 5 products in database:")
    for row in rows:
        db_id, barcode, name, price = row
        print(f"  DB_ID: {db_id}, Barcode: {barcode}, Name: {name}, Price: {price}")
    
    conn.close()
    print(f"\nID mapping saved to:")
    print(f"  - Ai_model/ai_to_db_id_mapping.json")
    print(f"  - Ai_model/db_to_ai_id_mapping.json")

if __name__ == '__main__':
    import_products() 