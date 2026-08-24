# Smart Fish Vending Machine - API Documentation

## Base URL
```
http://localhost:5050/api
```

## Authentication
Currently, no authentication is implemented. All endpoints are publicly accessible.

---

## Fish/Inventory Endpoints

### Get All Fish
```
GET /fish
```

**Response:**
```json
{
  "success": true,
  "data": [
    {
      "id": 1,
      "name": "Tilapia",
      "price": 12.50,
      "quantity": 20,
      "image_url": "https://...",
      "description": "Fresh tilapia...",
      "created_at": "2024-01-01T00:00:00",
      "updated_at": "2024-01-01T00:00:00"
    }
  ]
}
```

### Get Fish by ID
```
GET /fish/:id
```

### Add Fish (Admin)
```
POST /fish
Content-Type: application/json

{
  "name": "Tilapia",
  "price": 12.50,
  "quantity": 20,
  "image_url": "https://...",
  "description": "Fresh tilapia"
}
```

### Update Fish (Admin)
```
PUT /fish/:id
Content-Type: application/json

{
  "price": 14.00,
  "quantity": 25
}
```

### Delete Fish (Admin)
```
DELETE /fish/:id
```

---

## Cart/Checkout Endpoints

### Checkout
```
POST /checkout
Content-Type: application/json

{
  "items": [
    {
      "fishId": 1,
      "qty": 2
    }
  ],
  "customerName": "John Doe",
  "customerPhone": "+1234567890"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Order placed successfully",
  "orders": [
    {
      "id": "ORD1234",
      "fish_type": "Tilapia",
      "quantity": 2,
      "total_price": 25.00,
      "status": "processing",
      "payment_status": "pending"
    }
  ],
  "total_amount": 25.00
}
```

### Process Payment
```
POST /payment/process
Content-Type: application/json

{
  "orderId": "ORD1234"
}
```

---

## Order Endpoints

### Get All Orders
```
GET /orders?status=completed
```

### Get Order by ID
```
GET /order/:id
```

### Complete Order
```
POST /order/:id/complete
```

---

## Dashboard Endpoints

### Get Dashboard Statistics
```
GET /dashboard/stats
```

**Response:**
```json
{
  "success": true,
  "data": {
    "total_revenue": 1500.00,
    "total_orders": 120,
    "total_fish_sold": 450,
    "available_inventory": 65,
    "machine_online": true,
    "fish_types": 4,
    "machine_status": {
      "is_online": true,
      "temperature": 22.0,
      "humidity": 45.0
    }
  }
}
```

### Get Revenue Data
```
GET /dashboard/revenue
```

**Response:**
```json
{
  "success": true,
  "data": [
    {
      "date": "2024-01-01",
      "revenue": 150.00,
      "orders": 12
    }
  ]
}
```

### Get Fish Statistics
```
GET /dashboard/fish-stats
```

---

## ESP32 Endpoints

### Dispense Fish
```
POST /dispense
Content-Type: application/json

{
  "orderId": "ORD1234",
  "fishType": "Tilapia",
  "qty": 1
}
```

**Response:**
```json
{
  "success": true,
  "message": "Dispensing 1x Tilapia",
  "orderId": "ORD1234",
  "fishType": "Tilapia",
  "quantity": 1
}
```

### Update Machine Status
```
POST /esp32/status
Content-Type: application/json

{
  "online": true,
  "temperature": 22.0,
  "humidity": 45.0
}
```

---

## Health Check
```
GET /health
```

**Response:**
```json
{
  "status": "healthy",
  "service": "fish-vending-api"
}
```
