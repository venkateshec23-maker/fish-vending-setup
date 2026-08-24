import React from 'react';
import { Container, Row, Col, Card, Button, Badge } from 'react-bootstrap';
import { useCart } from '../components/CartContext';
import { getFish } from '../services/api';

const FishCard = ({ fish }) => {
  const { addToCart, cartItems } = useCart();

  const isInCart = cartItems.some(item => item.fishId === fish.id);

  const handleAddToCart = () => {
    if (fish.quantity > 0 && !isInCart) {
      addToCart(fish);
    }
  };

  return (
    <Card className="fish-card h-100 shadow-sm">
      <Card.Img
        variant="top"
        src={fish.image_url || 'https://via.placeholder.com/400x220?text=No+Image'}
        alt={fish.name}
        onError={(e) => {
          e.target.src = 'https://via.placeholder.com/400x220?text=Fish+Image';
        }}
      />
      <Card.Body className="d-flex flex-column">
        <Card.Title className="d-flex justify-content-between align-items-start">
          <span>{fish.name}</span>
          <Badge bg={fish.quantity > 0 ? 'success' : 'danger'}>
            {fish.quantity > 0 ? `${fish.quantity} left` : 'Out of Stock'}
          </Badge>
        </Card.Title>

        <Card.Text className="text-muted small flex-grow-1">
          {fish.description || 'Fresh fish from our sustainable sources.'}
        </Card.Text>

        <div className="mt-auto">
          <div className="d-flex justify-content-between align-items-center mb-3">
            <span className="price-tag">₹${fish.price.toFixed(2)}</span>
            <span className="text-muted">per unit</span>
          </div>

          <Button
            variant={isInCart ? 'success' : (fish.quantity > 0 ? 'primary' : 'secondary')}
            className="btn-add-cart"
            onClick={handleAddToCart}
            disabled={fish.quantity === 0 || isInCart}
          >
            {isInCart ? '✓ In Cart' : (fish.quantity > 0 ? '🛒 Add to Cart' : 'Out of Stock')}
          </Button>
        </div>
      </Card.Body>
    </Card>
  );
};

export default FishCard;
