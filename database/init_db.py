"""
Smart Fish Vending Machine - Database Initialization Script
Run this script to initialize/reset the SQLite database.
"""

import os
import sys

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from backend.app import create_app, db

def init_db():
    """Initialize the database with tables and seed data."""
    app = create_app()

    with app.app_context():
        print("Creating database tables...")
        db.create_all()

        # Migrate: add compartment column if missing
        try:
            with db.engine.connect() as conn:
                conn.execute(db.text('ALTER TABLE orders ADD COLUMN compartment INTEGER'))
                conn.commit()
                print("Added compartment column to orders table")
        except Exception:
            print("Compartment column already exists or migration not needed")

        print("Database tables created successfully!")

        # Verify tables
        from backend.models import Fish, Order, DailyRevenue, MachineStatus

        fish_count = Fish.query.count()
        order_count = Order.query.count()
        rev_count = DailyRevenue.query.count()
        status_count = MachineStatus.query.count()

        print(f"\nDatabase Statistics:")
        print(f"  Fish types: {fish_count}")
        print(f"  Orders: {order_count}")
        print(f"  Daily Revenue records: {rev_count}")
        print(f"  Machine Status records: {status_count}")

        if fish_count > 0:
            print("\nExisting Fish Inventory:")
            for fish in Fish.query.all():
                print(f"  - {fish.name}: ₹{fish.price} (Qty: {fish.quantity})")

        print("\nDatabase initialization complete!")

def reset_db():
    """Reset the database (drop and recreate all tables)."""
    app = create_app()
    
    with app.app_context():
        print("Dropping all tables...")
        db.drop_all()
        print("Creating database tables...")
        db.create_all()
        
        from backend.app import seed_initial_data
        seed_initial_data()
        
        print("Database reset complete!")

if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1] == 'reset':
        confirm = input("This will DELETE all data. Are you sure? (yes/no): ")
        if confirm.lower() == 'yes':
            reset_db()
        else:
            print("Reset cancelled.")
    else:
        init_db()
