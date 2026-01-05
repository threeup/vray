#include "card.h"
#include "ui.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <random>
#include <sstream>

namespace {

Vector2 applyMoveVector(const Vector2& start, const MoveVector& mv, Facing facing) {
    int dx = mv.lateral;
    int dy = mv.forward;

    switch (facing) {
    case Facing::North:
        break;
    case Facing::East: {
        int ndx = dy;
        int ndy = -dx;
        dx = ndx; dy = ndy;
        break;
    }
    case Facing::South:
        dx = -dx;
        dy = -dy;
        break;
    case Facing::West: {
        int ndx = -dy;
        int ndy = dx;
        dx = ndx; dy = ndy;
        break;
    }
    }

    return {start.x + static_cast<float>(dx), start.y + static_cast<float>(dy)};
}

bool positionOccupied(const std::vector<Entity>& entities, int selfId, const Vector2& target) {
    for (const auto& e : entities) {
        if (e.id == selfId) continue;
        if (static_cast<int>(std::round(e.position.x)) == static_cast<int>(std::round(target.x)) &&
            static_cast<int>(std::round(e.position.y)) == static_cast<int>(std::round(target.y))) {
            return true;
        }
    }
    return false;
}

const Card* findCard(const std::vector<Card>& hand, int cardId) {
    for (const auto& c : hand) {
        if (c.id == cardId) {
            return &c;
        }
    }
    return nullptr;
}

std::unordered_map<int, int> countTotals(const std::vector<Card>& cards) {
    std::unordered_map<int, int> totals;
    for (const auto& c : cards) {
        totals[c.id]++;
    }
    return totals;
}

} // namespace

void Hand::clear() {
    cards.clear();
    usage.clear();
}

void Hand::addCard(const Card& card) {
    cards.push_back(card);
}

void Hand::resetUsage() {
    usage.clear();
}

int Hand::totalCount(int cardId) const {
    int count = 0;
    for (const auto& c : cards) {
        if (c.id == cardId) {
            count++;
        }
    }
    return count;
}

int Hand::usedCount(int cardId) const {
    auto it = usage.find(cardId);
    return it == usage.end() ? 0 : it->second;
}

int Hand::availableCount(int cardId) const {
    int available = totalCount(cardId) - usedCount(cardId);
    return available < 0 ? 0 : available;
}

bool Hand::canPlay(int cardId) const {
    return availableCount(cardId) > 0;
}

bool Hand::markUsed(int cardId) {
    if (!canPlay(cardId)) {
        return false;
    }
    usage[cardId]++;
    return true;
}

bool Hand::unmarkUsed(int cardId) {
    auto it = usage.find(cardId);
    if (it == usage.end()) return false;
    if (it->second <= 1) {
        usage.erase(it);
    } else {
        it->second--;
    }
    return true;
}

std::vector<Card> Hand::availableCards() const {
    std::vector<Card> result;
    auto totals = countTotals(cards);
    std::unordered_map<int, int> emitted;
    for (const auto& c : cards) {
        int remaining = totals[c.id] - usedCount(c.id) - emitted[c.id];
        if (remaining > 0) {
            result.push_back(c);
            emitted[c.id]++;
        }
    }
    return result;
}

std::vector<int> Hand::availableCardIds() const {
    std::vector<int> ids;
    auto totals = countTotals(cards);
    std::unordered_map<int, int> emitted;
    for (const auto& c : cards) {
        int remaining = totals[c.id] - usedCount(c.id) - emitted[c.id];
        if (remaining > 0) {
            ids.push_back(c.id);
            emitted[c.id]++;
        }
    }
    return ids;
}

// T_051: Deck implementations
void Deck::clear() {
    cards.clear();
}

void Deck::addCard(const Card& card) {
    cards.push_back(card);
}

Card Deck::draw() {
    if (cards.empty()) {
        return Card{};  // Return empty card if deck is empty
    }
    Card drawn = cards.back();
    cards.pop_back();
    return drawn;
}

int Deck::remaining() const {
    return static_cast<int>(cards.size());
}

void Deck::shuffle(uint32_t seed) {
    std::mt19937 rng(seed);
    std::shuffle(cards.begin(), cards.end(), rng);
}

// T_057: Calculate mech stats based on assigned cards
MechStats calculateMechStats(int mechId, const Game& game) {
    MechStats stats;
    stats.baseHealth = 100;
    stats.currentHealth = 100;
    stats.attackBonus = 0;
    stats.defenseBonus = 0;
    stats.synergyText = "";
    
    // Get current health from entity
    for (const auto& entity : game.entities) {
        if (entity.id == mechId && entity.type == PLAYER) {
            stats.currentHealth = entity.health;
            break;
        }
    }
    
    // Calculate bonuses from assigned cards
    for (const auto& assignment : game.currentPlan.assignments) {
        if (assignment.mechId != mechId) continue;
        
        // Find the card
        const Card* card = nullptr;
        for (const auto& c : game.hand.cards) {
            if (c.id == assignment.cardId) {
                card = &c;
                break;
            }
        }
        if (!card) continue;
        
        // Get effect (mirrored or normal)
        const CardEffect& effect = assignment.useMirror ? card->mirroredEffect : card->effect;
        
        // Calculate bonuses
        if (effect.type == CardType::Damage) {
            stats.attackBonus += effect.damage;
        }
        if (effect.type == CardType::Heal) {
            stats.defenseBonus += effect.heal;
        }
    }
    
    // Generate synergy text
    if (stats.attackBonus > 0 && stats.defenseBonus > 0) {
        stats.synergyText = "Balanced: +" + std::to_string(stats.attackBonus + stats.defenseBonus);
    } else if (stats.attackBonus > 0) {
        stats.synergyText = "Aggressive: +" + std::to_string(stats.attackBonus);
    } else if (stats.defenseBonus > 0) {
        stats.synergyText = "Defensive: +" + std::to_string(stats.defenseBonus);
    }
    
    return stats;
}

CardEffect mirrorEffect(const CardEffect& effect) {
    if (effect.type != CardType::Move) {
        return effect;
    }
    CardEffect mirrored = effect;
    mirrored.move.lateral = -mirrored.move.lateral;
    if (mirrored.move.lateral == 0 && mirrored.move.forward != 0) {
        mirrored.move.forward = -mirrored.move.forward;
    }
    return mirrored;
}

GameState applyCard(const GameState& state, const Card& card, int playerId, bool useMirror) {
    GameState newState = state;
    const CardEffect& effect = useMirror ? card.mirroredEffect : card.effect;

    switch (effect.type) {
    case CardType::Move: {
        auto it = std::find_if(newState.entities.begin(), newState.entities.end(),
            [playerId](const Entity& e) { return e.id == playerId; });
        if (it != newState.entities.end()) {
            Vector2 target = applyMoveVector(it->position, effect.move, it->facing);
            int clampedX = static_cast<int>(std::round(target.x));
            int clampedY = static_cast<int>(std::round(target.y));
            clampedX = std::clamp(clampedX, 0, Grid::SIZE - 1);
            clampedY = std::clamp(clampedY, 0, Grid::SIZE - 1);
            target = {static_cast<float>(clampedX), static_cast<float>(clampedY)};

            if (!positionOccupied(newState.entities, it->id, target)) {
                it->position = target;
            }
        }
        break;
    }
    case CardType::Damage: {
        auto it = std::find_if(newState.entities.begin(), newState.entities.end(),
            [effect](const Entity& e) { return e.id == effect.targetEntityId; });
        if (it != newState.entities.end()) {
            it->health -= effect.damage;
            if (it->health < 0) it->health = 0;
        }
        break;
    }
    case CardType::Heal: {
        auto it = std::find_if(newState.entities.begin(), newState.entities.end(),
            [playerId](const Entity& e) { return e.id == playerId; });
        if (it != newState.entities.end()) {
            it->health += effect.heal;
            if (it->health > 100) it->health = 100;
        }
        break;
    }
    }

    return newState;
}

GameState applySequence(const GameState& state, const Sequence& sequence, int playerId) {
    GameState currentState = state;
    for (const auto& card : sequence) {
        currentState = applyCard(currentState, card, playerId);
    }
    return currentState;
}

bool TurnPlan::validate(const std::vector<Card>& hand, std::string* error) const {
    static const std::vector<int> emptyMechs;
    return validate(hand, emptyMechs, error);
}

bool TurnPlan::validate(const std::vector<Card>& hand, const std::vector<int>& mechIds, std::string* error) const {
    if (assignments.size() > 3) {
        if (error) *error = "Too many mech assignments (max 3)";
        return false;
    }
    std::unordered_set<int> mechSeen;
    std::unordered_map<int, int> handCounts;
    for (const auto& c : hand) {
        handCounts[c.id]++;
    }

    std::unordered_set<int> mechRoster(mechIds.begin(), mechIds.end());

    std::unordered_map<int, int> usedCounts;
    for (const auto& a : assignments) {
        if (a.mechId < 0) {
            if (error) *error = "Invalid mech id";
            return false;
        }
        if (!mechRoster.empty() && mechRoster.count(a.mechId) == 0) {
            if (error) *error = "Mech id not present in roster";
            return false;
        }
        if (!mechSeen.insert(a.mechId).second) {
            if (error) *error = "Duplicate mech assignment";
            return false;
        }
        if (a.cardId < 0 || handCounts.find(a.cardId) == handCounts.end()) {
            if (error) *error = "Card not available in hand";
            return false;
        }
        usedCounts[a.cardId]++;
        if (usedCounts[a.cardId] > handCounts[a.cardId]) {
            if (error) *error = "Card used more times than available";
            return false;
        }
    }
    return true;
}

GameState TurnPlan::apply(const GameState& state, const std::vector<Card>& hand, const Grid& grid) const {
    GameState current = state;
    for (const auto& a : assignments) {
        const Card* c = findCard(hand, a.cardId);
        if (!c) {
            continue;
        }
        current = applyCard(current, *c, a.mechId, a.useMirror);
    }
    current.grid = grid;
    return current;
}

TurnPlan buildRandomPlan(const std::vector<int>& mechIds, Hand& hand, uint32_t seed, float mirrorChance) {
    TurnPlan plan;
    std::mt19937 rng(seed);

    for (int mechId : mechIds) {
        auto options = hand.availableCardIds();
        if (options.empty()) {
            break;
        }
        std::uniform_int_distribution<size_t> pick(0, options.size() - 1);
        int chosenCardId = options[pick(rng)];
        bool useMirror = std::bernoulli_distribution(mirrorChance)(rng);
        if (hand.markUsed(chosenCardId)) {
            plan.assignments.push_back({mechId, chosenCardId, useMirror});
        }
    }

    return plan;
}

std::string cardTypeToString(CardType t) {
    switch (t) {
    case CardType::Move: return "Move";
    case CardType::Damage: return "Damage";
    case CardType::Heal: return "Heal";
    }
    return "Move";
}

CardType cardTypeFromString(const std::string& s) {
    if (s == "Damage") return CardType::Damage;
    if (s == "Heal") return CardType::Heal;
    return CardType::Move;
}

namespace {

std::string boolString(bool v) { return v ? "true" : "false"; }

bool extractString(const std::string& src, const std::string& key, std::string& out) {
    std::string pattern = "\"" + key + "\"";
    size_t pos = src.find(pattern);
    if (pos == std::string::npos) return false;
    pos = src.find('"', pos + pattern.size());
    if (pos == std::string::npos) return false;
    size_t start = pos + 1;
    size_t end = src.find('"', start);
    if (end == std::string::npos) return false;
    out = src.substr(start, end - start);
    return true;
}

bool extractInt(const std::string& src, const std::string& key, int& out) {
    std::string pattern = "\"" + key + "\"";
    size_t pos = src.find(pattern);
    if (pos == std::string::npos) return false;
    pos = src.find(':', pos);
    if (pos == std::string::npos) return false;
    pos++;
    while (pos < src.size() && src[pos] == ' ') pos++;
    try {
        out = std::stoi(src.substr(pos));
        return true;
    } catch (...) {
        return false;
    }
}

bool extractBool(const std::string& src, const std::string& key, bool& out) {
    std::string pattern = "\"" + key + "\"";
    size_t pos = src.find(pattern);
    if (pos == std::string::npos) return false;
    pos = src.find(':', pos);
    if (pos == std::string::npos) return false;
    pos++;
    while (pos < src.size() && src[pos] == ' ') pos++;
    if (src.compare(pos, 4, "true") == 0) { out = true; return true; }
    if (src.compare(pos, 5, "false") == 0) { out = false; return true; }
    return false;
}

} // namespace

std::string serializeCard(const Card& card) {
    std::ostringstream oss;
    oss << "{\"id\":" << card.id
        << ",\"name\":\"" << card.name << "\""
        << ",\"type\":\"" << cardTypeToString(card.type) << "\""
        << ",\"move\":{\"forward\":" << card.effect.move.forward << ",\"lateral\":" << card.effect.move.lateral << "}"
        << ",\"damage\":" << card.effect.damage
        << ",\"heal\":" << card.effect.heal
        << "}";
    return oss.str();
}

bool deserializeCard(const std::string& json, Card& out) {
    int id = 0; std::string name; std::string typeStr;
    if (!extractInt(json, "id", id)) return false;
    if (!extractString(json, "name", name)) return false;
    if (!extractString(json, "type", typeStr)) return false;
    int fwd = 0, lat = 0, dmg = 0, heal = 0;
    extractInt(json, "forward", fwd);
    extractInt(json, "lateral", lat);
    extractInt(json, "damage", dmg);
    extractInt(json, "heal", heal);

    out.id = id;
    out.name = name;
    out.type = cardTypeFromString(typeStr);
    out.effect.type = out.type;
    out.effect.move.forward = fwd;
    out.effect.move.lateral = lat;
    out.effect.damage = dmg;
    out.effect.heal = heal;
    out.mirroredEffect = mirrorEffect(out.effect);
    return true;
}

std::string serializeHand(const Hand& hand) {
    std::ostringstream oss;
    oss << "{\"cards\":[";
    for (size_t i = 0; i < hand.cards.size(); ++i) {
        if (i > 0) oss << ",";
        oss << serializeCard(hand.cards[i]);
    }
    oss << "]}";
    return oss.str();
}

bool deserializeHand(const std::string& json, Hand& out) {
    out.clear();
    size_t start = json.find('[');
    size_t end = json.rfind(']');
    if (start == std::string::npos || end == std::string::npos || end <= start) return false;
    size_t pos = start + 1;
    while (pos < end) {
        size_t objStart = json.find('{', pos);
        if (objStart == std::string::npos || objStart >= end) break;
        size_t objEnd = json.find('}', objStart);
        if (objEnd == std::string::npos || objEnd > end) return false;
        Card c;
        if (!deserializeCard(json.substr(objStart, objEnd - objStart + 1), c)) return false;
        out.addCard(c);
        pos = objEnd + 1;
        size_t comma = json.find(',', pos);
        if (comma != std::string::npos && comma < end) pos = comma + 1;
    }
    out.resetUsage();
    return true;
}

std::string serializeTurnPlan(const TurnPlan& plan) {
    std::ostringstream oss;
    oss << "{\"assignments\":[";
    for (size_t i = 0; i < plan.assignments.size(); ++i) {
        const auto& a = plan.assignments[i];
        if (i > 0) oss << ",";
        oss << "{\"mechId\":" << a.mechId
            << ",\"cardId\":" << a.cardId
            << ",\"useMirror\":" << boolString(a.useMirror) << "}";
    }
    oss << "]}";
    return oss.str();
}

bool deserializeTurnPlan(const std::string& json, TurnPlan& out) {
    out.assignments.clear();
    size_t start = json.find('[');
    size_t end = json.rfind(']');
    if (start == std::string::npos || end == std::string::npos || end <= start) return false;
    size_t pos = start + 1;
    while (pos < end) {
        size_t objStart = json.find('{', pos);
        if (objStart == std::string::npos || objStart >= end) break;
        size_t objEnd = json.find('}', objStart);
        if (objEnd == std::string::npos || objEnd > end) return false;
        std::string slice = json.substr(objStart, objEnd - objStart + 1);
        PlanAssignment a;
        int mech = -1, cardId = -1; bool mirror = false;
        if (!extractInt(slice, "mechId", mech)) return false;
        if (!extractInt(slice, "cardId", cardId)) return false;
        extractBool(slice, "useMirror", mirror);
        a.mechId = mech;
        a.cardId = cardId;
        a.useMirror = mirror;
        out.assignments.push_back(a);
        pos = objEnd + 1;
        size_t comma = json.find(',', pos);
        if (comma != std::string::npos && comma < end) pos = comma + 1;
    }
    return true;
}