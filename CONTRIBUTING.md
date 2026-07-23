# Contributing to Real-Time Ring Buffer

Contributions are welcome! Please follow these guidelines.

## Getting Started

1. Fork the repository
2. Clone your fork
3. Create a feature branch: `git checkout -b feature/your-feature`
4. Make your changes
5. Test thoroughly
6. Submit a pull request

## Code Style

- Use 4 spaces for indentation
- Follow Google C++ style guide (mostly)
- Use meaningful variable names
- Add comments for complex logic
- Keep lines under 100 characters

## Testing

Before submitting:

```bash
cmake -B build
cmake --build build
cmake --build build --target test
```

## Documentation

- Update README.md if adding features
- Document public APIs in code comments
- Add examples in docs/ for significant features

## Performance

- Run benchmarks before/after changes
- Avoid breaking existing performance guarantees
- Document any performance tradeoffs

## Issues

When reporting bugs, include:
- Operating system and CPU
- Compiler and version
- Reproduction steps
- Expected vs. actual behavior

Thank you for contributing!
