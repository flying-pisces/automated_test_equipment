# Contributing to Automated Test Equipment Controller

We love your input! We want to make contributing to this project as easy and transparent as possible, whether it's:

- Reporting a bug
- Discussing the current state of the code
- Submitting a fix
- Proposing new features
- Adding new equipment support
- Becoming a maintainer

## Development Process

We use GitHub to host code, to track issues and feature requests, as well as accept pull requests.

### Pull Requests

Pull requests are the best way to propose changes to the codebase. We actively welcome your pull requests:

1. Fork the repo and create your branch from `main`.
2. If you've added code that should be tested, add tests.
3. If you've changed APIs, update the documentation.
4. Ensure the test suite passes.
5. Make sure your code lints.
6. Issue that pull request!

## Adding New Equipment

### 1. Create Equipment Class

Create a new equipment class that inherits from `BaseEquipment`:

```python
from base_equipment import BaseEquipment, IOType, EquipmentStatus

class MyNewEquipment(BaseEquipment):
    def __init__(self, connection_params: Dict[str, Any]):
        super().__init__("My Equipment", IOType.ETHERNET, connection_params)
        # Equipment-specific initialization
    
    def connect(self) -> bool:
        # Implement connection logic
        pass
    
    # Implement all other abstract methods
    def disconnect(self) -> bool:
        pass
    
    def is_connected(self) -> bool:
        pass
    
    # ... etc
```

### 2. Add to Equipment Factory

Register your equipment in `equipment_controller.py`:

```python
EQUIPMENT_TYPES = {
    'my_new_equipment': MyNewEquipment,
    # ... existing equipment
}
```

### 3. Create Tests

Add test cases for your equipment:

```python
import unittest
from your_module import MyNewEquipment

class TestMyNewEquipment(unittest.TestCase):
    def setUp(self):
        self.equipment = MyNewEquipment({"param": "value"})
    
    def test_connect(self):
        # Test connection logic
        pass
```

### 4. Update Documentation

- Update README.md with your equipment in the supported list
- Add configuration examples
- Create equipment-specific documentation if needed

## Code Style

### Python Style Guide

We follow PEP 8 with some specific guidelines:

- **Line Length**: 100 characters maximum
- **Imports**: Group standard library, third-party, and local imports
- **Documentation**: Use docstrings for all public methods
- **Type Hints**: Use type hints for function parameters and return values

### Equipment Class Guidelines

1. **Inherit from BaseEquipment**: All equipment must inherit from the base class
2. **Implement All Abstract Methods**: Don't leave abstract methods unimplemented
3. **Error Handling**: Use try-catch blocks and set `self.last_error` appropriately
4. **Status Updates**: Update `self.status` using `_set_status()` method
5. **Configuration**: Support configuration via `config` dictionary
6. **Thread Safety**: Ensure thread-safe operations for GUI compatibility

### Example Equipment Structure

```python
class ExampleEquipment(BaseEquipment):
    def __init__(self, connection_params):
        super().__init__("Example Equipment", IOType.ETHERNET, connection_params)
        
        # Equipment-specific default configuration
        self.default_config.update({
            'parameter1': 'default_value',
            'parameter2': 100,
            'parameter3': True
        })
        self.config.update(self.default_config)
        
        self._connection = None
    
    def connect(self) -> bool:
        try:
            self._set_status(EquipmentStatus.CONNECTING)
            # Connection logic here
            self._connection = create_connection(self.connection_params)
            self._set_status(EquipmentStatus.READY)
            return True
        except Exception as e:
            self._set_status(EquipmentStatus.ERROR, f"Connection failed: {str(e)}")
            return False
    
    # ... implement other methods
```

## Testing

### Running Tests

```bash
# Run all tests
python -m pytest tests/

# Run specific test file
python -m pytest tests/test_equipment.py

# Run with coverage
python -m pytest --cov=. tests/
```

### Test Guidelines

1. **Test All Public Methods**: Every public method should have tests
2. **Mock External Dependencies**: Use mocks for hardware connections
3. **Test Error Conditions**: Test both success and failure scenarios
4. **Use Descriptive Names**: Test method names should describe what they test

### Example Test

```python
import unittest
from unittest.mock import Mock, patch
from my_equipment import MyEquipment

class TestMyEquipment(unittest.TestCase):
    def setUp(self):
        self.equipment = MyEquipment({"address": "192.168.1.100"})
    
    @patch('my_equipment.create_connection')
    def test_connect_success(self, mock_connection):
        mock_connection.return_value = Mock()
        
        result = self.equipment.connect()
        
        self.assertTrue(result)
        self.assertTrue(self.equipment.is_connected())
        self.assertEqual(self.equipment.status, EquipmentStatus.READY)
    
    @patch('my_equipment.create_connection')
    def test_connect_failure(self, mock_connection):
        mock_connection.side_effect = Exception("Connection failed")
        
        result = self.equipment.connect()
        
        self.assertFalse(result)
        self.assertFalse(self.equipment.is_connected())
        self.assertEqual(self.equipment.status, EquipmentStatus.ERROR)
```

## Bug Reports

We use GitHub issues to track public bugs. Report a bug by [opening a new issue](https://github.com/yourusername/automated_test_equipment/issues).

**Great Bug Reports** tend to have:

- A quick summary and/or background
- Steps to reproduce
  - Be specific!
  - Give sample code if you can
- What you expected would happen
- What actually happens
- Notes (possibly including why you think this might be happening, or stuff you tried that didn't work)

## Feature Requests

We use GitHub issues to track feature requests as well. When requesting a feature:

- Explain the motivation - why do you need this feature?
- Describe the proposed solution
- Consider alternative solutions
- Provide examples of how the feature would be used

## License

By contributing, you agree that your contributions will be licensed under its MIT License.

## References

This document was adapted from the open-source contribution guidelines for [Facebook's Draft](https://github.com/facebook/draft-js/blob/a9316a723f9e918afde44dea68b5f9f39b7d9b00/CONTRIBUTING.md).

## Development Setup

### Environment Setup

```bash
# Clone your fork
git clone https://github.com/yourusername/automated_test_equipment.git
cd automated_test_equipment

# Create virtual environment
python -m venv venv
source venv/bin/activate  # Linux/macOS
# venv\Scripts\activate   # Windows

# Install development dependencies
pip install -r requirements.txt
pip install -r requirements-dev.txt

# Install pre-commit hooks
pre-commit install
```

### Making Changes

1. Create a new branch: `git checkout -b feature/my-new-feature`
2. Make your changes
3. Add tests for your changes
4. Run tests: `python -m pytest`
5. Run linting: `flake8 .`
6. Run type checking: `mypy .`
7. Commit your changes: `git commit -am 'Add some feature'`
8. Push to the branch: `git push origin feature/my-new-feature`
9. Submit a pull request

### Code Review Process

1. All submissions require review before merging
2. Reviews focus on:
   - Code quality and style
   - Test coverage
   - Documentation completeness
   - Compatibility with existing equipment
3. Address reviewer feedback
4. Maintainer will merge once approved

## Getting Help

- Check existing [issues](https://github.com/yourusername/automated_test_equipment/issues)
- Join our [discussions](https://github.com/yourusername/automated_test_equipment/discussions)
- Read the [documentation](https://github.com/yourusername/automated_test_equipment/wiki)

Thank you for contributing! 🎉