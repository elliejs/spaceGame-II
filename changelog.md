# SPACEGAME II -- Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
- User keymaps for Colemak, Dvorak, etc
- Friend sharing and waypointing
- Better planet gen
    - More planets, revolution, rotation, and orbital stellar systems

## [Bugs]
- User session can hang in specific conditions
    - User can still play, move, and keep up to date, but cannot log out by conventional means. Client zombifies on server when force closed by the client
- If more than one user is used in the database, data is mangled
    - Causes black screens and dead controls

## [0.1.0] - 2023-03-15

### Added
- Users' attitude now saved along side absolute universal position
- Link to this document where the pseudo-changelist used to live in the [readme](readme.md)
- Prismatic spaceships
- This changelog

### Changed
- Major [readme](readme.md) overhaul

### Fixed
- SGVec4D_Mult_SGVec4D no longer contains sign based bugs

### Removed
- Pseudo-changelist from the [readme](readme.md) moved here
