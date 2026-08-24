import React from 'react';
import { Container, Navbar, Nav, Button, Badge } from 'react-bootstrap';
import { LinkContainer } from 'react-router-dom';
import { useCart } from '../components/CartContext';

const NavigationBar = () => {
  const { getCartCount, toggleCart } = useCart();
  const cartCount = getCartCount();

  return (
    <Navbar bg="dark" variant="dark" expand="lg" className="mb-4 shadow-sm">
      <Container>
        <LinkContainer to="/">
          <Navbar.Brand>
            🐟 Smart Fish Vending
          </Navbar.Brand>
        </LinkContainer>

        <Navbar.Toggle aria-controls="basic-navbar-nav" />

        <Navbar.Collapse id="basic-navbar-nav">
          <Nav className="me-auto">
            <LinkContainer to="/">
              <Nav.Link>Browse Fish</Nav.Link>
            </LinkContainer>
            <LinkContainer to="/orders">
              <Nav.Link>My Orders</Nav.Link>
            </LinkContainer>
            <LinkContainer to="/admin">
              <Nav.Link>Admin Dashboard</Nav.Link>
            </LinkContainer>
          </Nav>

          <Button
            variant="outline-light"
            onClick={toggleCart}
            className="position-relative"
          >
            🛒 Cart
            {cartCount > 0 && (
              <Badge
                bg="danger"
                pill
                className="position-absolute top-0 start-100 translate-middle"
              >
                {cartCount}
              </Badge>
            )}
          </Button>
        </Navbar.Collapse>
      </Container>
    </Navbar>
  );
};

export default NavigationBar;
