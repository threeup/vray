#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <cstdint>
#include "entity.h"
#include "grid.h"

enum class CardType {
    Move,
    Damage,
    Heal
};

struct MoveVector {
    int forward = 0; // +Y forward, -Y backward
    int lateral = 0; // +X right, -X left
};

struct CardEffect {
    CardType type = CardType::Move;
    MoveVector move{};
    int targetEntityId = -1; // Used for damage/heal
    int damage = 0;
    int heal = 0;
};

struct Card {
    int id = 0;
    std::string name;
    CardType type = CardType::Move;
    CardEffect effect{};
    CardEffect mirroredEffect{}; // precomputed for faster use
};

struct Hand {
    std::vector<Card> cards;
    std::unordered_map<int, int> usage;

    void clear();
    void addCard(const Card& card);
    void resetUsage();
    int totalCount(int cardId) const;
    int usedCount(int cardId) const;
    int availableCount(int cardId) const;
    bool canPlay(int cardId) const;
    bool markUsed(int cardId);
    bool unmarkUsed(int cardId);
    std::vector<Card> availableCards() const;
    std::vector<int> availableCardIds() const;
};

struct PlanAssignment {
    int mechId = -1;
    int cardId = -1;
    bool useMirror = false;
};

using Sequence = std::vector<Card>;

struct GameState {
    Grid grid;
    std::vector<Entity> entities;
    int currentTurn = 0;
};

CardEffect mirrorEffect(const CardEffect& effect);
std::string cardTypeToString(CardType t);
CardType cardTypeFromString(const std::string& s);

struct TurnPlan {
    std::vector<PlanAssignment> assignments;
    bool validate(const std::vector<Card>& hand, std::string* error = nullptr) const;
    bool validate(const std::vector<Card>& hand, const std::vector<int>& mechIds, std::string* error = nullptr) const;
    GameState apply(const GameState& state, const std::vector<Card>& hand, const Grid& grid) const;
};

TurnPlan buildRandomPlan(const std::vector<int>& mechIds, Hand& hand, uint32_t seed, float mirrorChance = 0.5f);

// Lightweight JSON-ish helpers (for tests/tools; no persistence layer yet).
std::string serializeCard(const Card& card);
bool deserializeCard(const std::string& json, Card& out);
std::string serializeHand(const Hand& hand);
bool deserializeHand(const std::string& json, Hand& out);
std::string serializeTurnPlan(const TurnPlan& plan);
bool deserializeTurnPlan(const std::string& json, TurnPlan& out);

GameState applyCard(const GameState& state, const Card& card, int playerId, bool useMirror = false);
GameState applySequence(const GameState& state, const Sequence& sequence, int playerId);