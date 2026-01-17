# Modular Interaction Framework (Unreal Engine)

A data-driven interaction system built in Unreal Engine that supports
state-based interactions, requirements, NPCs, and designer-configurable behavior.


## Demo Overview

The included demo level is designed as a single-room apartment with everyday objects demonstrating the various capabilities of the framework, such as state transitions, requirement gating, focus-driven behaviour, one-sided NPC interaction, and pick-up based progression.


## Key Features

- Interface-based interaction system (`IInteractable`)
- State-driven interaction logic with per-state requirements
- Support for multiple simultaneous requirements
- Focus-based state transitions (`OnFocusStart/OnFocusEnd`)
- Optional display of missing requirements per state
- NPC interactions using shared interaction interface
- Pickupable objects that grant keys and destroy themselves
- Modular UI prompts decoupled from interaction logic
- Debug and validation utilities for development
- Press and hold interaction options


## Demo Walkthrough

1. **Basic Two-State Interaction**
Demonstrates a simple interactable with no requirements, switching between two states.
The lamp. Switches between the on and off states.

2. **Locked Interaction → Free Interaction**
An object requires a key in its initial state. Once unlocked, it transitions into a reusable two-state interaction.
Vinyl player. Initially requires a vinyl record. Once the record is placed, it switches between on and off states.

3. **Focus-Driven Reveal**
An object starts with no prompt or interaction. When the player acquires a hidden key, later when they focus on the object, triggers a state change that enables interaction.
Water bottle that can only be picked up after the plant requests it.




4. **Multiple Requirements**
An interactable requires two keys simultaneously. After use, it transitions into a non-functional state.
The ironing board. It requires both the shirt and the iron to be used. It becomes un-interactable after use.

5. **Promptless Interaction**
An interactable that does not display a prompt but can still be used.
The cookie jar. Has a single state and can always be interacted with.

6. **NPC Interaction**
An NPC guides the player and grants progression keys. NPCs use the same interaction framework as world objects.
The plant, guiding the player on how to water it. Has multiple states that each have a success and fail response depending on whether the player has fulfilled the requirement for that state.

7. **Pickup Interactions**
Pickupable objects grant keys and destroy themselves after use.
Examples to this interactable type are the vinyl record, the iron and the shirt.

8. **Single-State Interaction With No Requirements**
Always allows interaction due to not having any requirements, and does not switch between states.
The keyboard and the mouse.

9. **Mixed Behaviour**
Many other interaction-state-requirement configurations can be made with more complex behaviour. 
An example of this is the faucet. It initially behaves as a “Basic Two-State Interaction”, turning on and off. If the faucet is already on or if it is turned on while the player possesses an “Empty Water Bottle” key, it transitions into the “Fill” state, allowing the bottle to be filled via a hold interaction. 


## Architecture Overview

The framework is built around a component and interface-driven architecture:

- `InteractionComponent`
Handles player-side interaction logic and focus detection.

- `IInteractable`
Interface implemented by all interactable actors.

- `InteractableActorBase and InteractableNpcActorBase`
Base class that implements the IInteractable interface.

- `InteractionDataAsset and NpcInteractionDataAsset`
Defines interaction states, requirements, and prompt data.

- `KeyringComponent`
Stores acquired keys and evaluates requirements.

- UI Widgets
Interaction prompts and NPC speech bubbles are driven by data, not hardcoded logic.


## Design Goals and Scope

This project is intentionally scoped as an interaction framework, not a dialogue
or quest system.

Out of scope by design:
- Branching dialogue trees
- Narrative scripting
- A full game loop

The focus is on creating a clean, extensible interaction foundation that can
be reused or extended in larger projects.


## Extensibility

The system is designed to be easily extended by:
- Adding new interaction states via Data Assets
- Creating new interactables by implementing `IInteractable`
- Customizing per-interactable behaviour through BP hooks (Interaction success or failure) 
- Customizing prompts and UI behavior without code changes


## Author

Developed by Emir Şahin.

This project was created as a portfolio piece to demonstrate
gameplay and systems programming practices in Unreal Engine.