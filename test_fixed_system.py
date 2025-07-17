#!/usr/bin/env python3
import requests
import json
import time

def test_ai_system():
    """Test the AI recommendation system with ID mapping."""
    base_url = "http://127.0.0.1:5001"
    
    print("🧪 Testing AI System with ID Mapping Fix")
    print("=" * 50)
    
    # Wait for server to be ready
    print("⏳ Waiting for AI server to start...")
    time.sleep(5)
    
    # Test 1: User query recommendation
    print("\n1️⃣ Testing User Query Recommendation")
    print("-" * 30)
    query_data = {"query": "推荐一些运动用品"}
    try:
        response = requests.post(f"{base_url}/ask", json=query_data, timeout=10)
        if response.status_code == 200:
            result = response.json()
            print("✅ User Query Test Successful!")
            print(f"AI Response: {result.get('response', 'No response')}")
            print(f"Recommended Product IDs: {result.get('products', [])}")
        else:
            print(f"❌ User Query Test Failed: HTTP {response.status_code}")
    except Exception as e:
        print(f"❌ User Query Test Error: {e}")
    
    # Test 2: Cart-based recommendation
    print("\n2️⃣ Testing Cart-based Recommendation")
    print("-" * 30)
    # Test with AI IDs (these should be converted by the system)
    cart_data = {"cart_items": ["SPORTS001", "ELEC001"]}
    try:
        response = requests.post(f"{base_url}/recommend", json=cart_data, timeout=10)
        if response.status_code == 200:
            result = response.json()
            print("✅ Cart Recommendation Test Successful!")
            print(f"AI Response: {result.get('response', 'No response')}")
            print(f"Recommended Product IDs: {result.get('products', [])}")
        else:
            print(f"❌ Cart Recommendation Test Failed: HTTP {response.status_code}")
    except Exception as e:
        print(f"❌ Cart Recommendation Test Error: {e}")
    
    # Test 3: Check ID mappings
    print("\n3️⃣ Testing ID Mapping Files")
    print("-" * 30)
    try:
        with open('Ai_model/ai_to_db_id_mapping.json', 'r') as f:
            ai_to_db = json.load(f)
        with open('Ai_model/db_to_ai_id_mapping.json', 'r') as f:
            db_to_ai = json.load(f)
        
        print("✅ ID Mapping Files Loaded Successfully!")
        print(f"AI to DB mappings: {len(ai_to_db)} entries")
        print(f"DB to AI mappings: {len(db_to_ai)} entries")
        
        # Show some examples
        print("\n📋 Sample Mappings:")
        for i, (ai_id, db_id) in enumerate(list(ai_to_db.items())[:5]):
            print(f"  {ai_id} → {db_id}")
        
    except Exception as e:
        print(f"❌ ID Mapping Test Error: {e}")
    
    # Test 4: Database verification
    print("\n4️⃣ Testing Database Content")
    print("-" * 30)
    try:
        import sqlite3
        conn = sqlite3.connect('pos_database.db')
        cursor = conn.cursor()
        cursor.execute('SELECT COUNT(*) FROM Products')
        count = cursor.fetchone()[0]
        
        cursor.execute('SELECT product_id, barcode, name FROM Products LIMIT 3')
        samples = cursor.fetchall()
        
        print("✅ Database Test Successful!")
        print(f"Total products in database: {count}")
        print("\n📋 Sample Products:")
        for db_id, barcode, name in samples:
            print(f"  DB_ID: {db_id}, Barcode: {barcode}, Name: {name}")
        
        conn.close()
    except Exception as e:
        print(f"❌ Database Test Error: {e}")
    
    print("\n🎯 Testing Complete!")
    print("=" * 50)
    print("If all tests passed, the ID mapping system is working correctly.")
    print("The C++ application should now be able to:")
    print("  • Add recommended items to cart")
    print("  • Display AI responses properly")
    print("  • Handle cart-based recommendations")

if __name__ == '__main__':
    test_ai_system() 