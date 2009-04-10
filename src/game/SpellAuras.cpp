/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2009 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "UpdateMask.h"
#include "World.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "Unit.h"
#include "Spell.h"
#include "DynamicObject.h"
#include "Group.h"
#include "UpdateData.h"
#include "ObjectAccessor.h"
#include "Policies/SingletonImp.h"
#include "Totem.h"
#include "Creature.h"
#include "Formulas.h"
#include "BattleGround.h"
#include "OutdoorPvP.h"
#include "OutdoorPvPMgr.h"
#include "CreatureAI.h"
#include "ScriptCalls.h"
#include "Util.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"

pAuraHandler AuraHandler[TOTAL_AURAS]=
{
    &AuraEffect::HandleNULL,                                      //  0 SPELL_AURA_NONE
    &AuraEffect::HandleBindSight,                                 //  1 SPELL_AURA_BIND_SIGHT
    &AuraEffect::HandleModPossess,                                //  2 SPELL_AURA_MOD_POSSESS
    &AuraEffect::HandlePeriodicDamage,                            //  3 SPELL_AURA_PERIODIC_DAMAGE
    &AuraEffect::HandleAuraDummy,                                 //  4 SPELL_AURA_DUMMY
    &AuraEffect::HandleModConfuse,                                //  5 SPELL_AURA_MOD_CONFUSE
    &AuraEffect::HandleModCharm,                                  //  6 SPELL_AURA_MOD_CHARM
    &AuraEffect::HandleModFear,                                   //  7 SPELL_AURA_MOD_FEAR
    &AuraEffect::HandlePeriodicHeal,                              //  8 SPELL_AURA_PERIODIC_HEAL
    &AuraEffect::HandleModAttackSpeed,                            //  9 SPELL_AURA_MOD_ATTACKSPEED
    &AuraEffect::HandleModThreat,                                 // 10 SPELL_AURA_MOD_THREAT
    &AuraEffect::HandleModTaunt,                                  // 11 SPELL_AURA_MOD_TAUNT
    &AuraEffect::HandleAuraModStun,                               // 12 SPELL_AURA_MOD_STUN
    &AuraEffect::HandleModDamageDone,                             // 13 SPELL_AURA_MOD_DAMAGE_DONE
    &AuraEffect::HandleNoImmediateEffect,                         // 14 SPELL_AURA_MOD_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         // 15 SPELL_AURA_DAMAGE_SHIELD    implemented in Unit::DoAttackDamage
    &AuraEffect::HandleModStealth,                                // 16 SPELL_AURA_MOD_STEALTH
    &AuraEffect::HandleNoImmediateEffect,                         // 17 SPELL_AURA_MOD_STEALTH_DETECT
    &AuraEffect::HandleInvisibility,                              // 18 SPELL_AURA_MOD_INVISIBILITY
    &AuraEffect::HandleInvisibilityDetect,                        // 19 SPELL_AURA_MOD_INVISIBILITY_DETECTION
    &AuraEffect::HandleAuraModTotalHealthPercentRegen,            // 20 SPELL_AURA_OBS_MOD_HEALTH
    &AuraEffect::HandleAuraModTotalEnergyPercentRegen,              // 21 SPELL_AURA_OBS_MOD_ENERGY
    &AuraEffect::HandleAuraModResistance,                         // 22 SPELL_AURA_MOD_RESISTANCE
    &AuraEffect::HandlePeriodicTriggerSpell,                      // 23 SPELL_AURA_PERIODIC_TRIGGER_SPELL
    &AuraEffect::HandlePeriodicEnergize,                          // 24 SPELL_AURA_PERIODIC_ENERGIZE
    &AuraEffect::HandleAuraModPacify,                             // 25 SPELL_AURA_MOD_PACIFY
    &AuraEffect::HandleAuraModRoot,                               // 26 SPELL_AURA_MOD_ROOT
    &AuraEffect::HandleAuraModSilence,                            // 27 SPELL_AURA_MOD_SILENCE
    &AuraEffect::HandleNoImmediateEffect,                         // 28 SPELL_AURA_REFLECT_SPELLS        implement in Unit::SpellHitResult
    &AuraEffect::HandleAuraModStat,                               // 29 SPELL_AURA_MOD_STAT
    &AuraEffect::HandleAuraModSkill,                              // 30 SPELL_AURA_MOD_SKILL
    &AuraEffect::HandleAuraModIncreaseSpeed,                      // 31 SPELL_AURA_MOD_INCREASE_SPEED
    &AuraEffect::HandleAuraModIncreaseMountedSpeed,               // 32 SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED
    &AuraEffect::HandleAuraModDecreaseSpeed,                      // 33 SPELL_AURA_MOD_DECREASE_SPEED
    &AuraEffect::HandleAuraModIncreaseHealth,                     // 34 SPELL_AURA_MOD_INCREASE_HEALTH
    &AuraEffect::HandleAuraModIncreaseEnergy,                     // 35 SPELL_AURA_MOD_INCREASE_ENERGY
    &AuraEffect::HandleAuraModShapeshift,                         // 36 SPELL_AURA_MOD_SHAPESHIFT
    &AuraEffect::HandleAuraModEffectImmunity,                     // 37 SPELL_AURA_EFFECT_IMMUNITY
    &AuraEffect::HandleAuraModStateImmunity,                      // 38 SPELL_AURA_STATE_IMMUNITY
    &AuraEffect::HandleAuraModSchoolImmunity,                     // 39 SPELL_AURA_SCHOOL_IMMUNITY
    &AuraEffect::HandleAuraModDmgImmunity,                        // 40 SPELL_AURA_DAMAGE_IMMUNITY
    &AuraEffect::HandleAuraModDispelImmunity,                     // 41 SPELL_AURA_DISPEL_IMMUNITY
    &AuraEffect::HandleAuraProcTriggerSpell,                      // 42 SPELL_AURA_PROC_TRIGGER_SPELL  implemented in Unit::ProcDamageAndSpellFor and Unit::HandleProcTriggerSpell
    &AuraEffect::HandleNoImmediateEffect,                         // 43 SPELL_AURA_PROC_TRIGGER_DAMAGE implemented in Unit::ProcDamageAndSpellFor
    &AuraEffect::HandleAuraTrackCreatures,                        // 44 SPELL_AURA_TRACK_CREATURES
    &AuraEffect::HandleAuraTrackResources,                        // 45 SPELL_AURA_TRACK_RESOURCES
    &AuraEffect::HandleUnused,                                    // 46 SPELL_AURA_46 (used in test spells 54054 and 54058, and spell 48050) (3.0.8a)
    &AuraEffect::HandleAuraModParryPercent,                       // 47 SPELL_AURA_MOD_PARRY_PERCENT
    &AuraEffect::HandleNULL,                                      // 48 SPELL_AURA_48 spell Napalm (area damage spell with additional delayed damage effect)
    &AuraEffect::HandleAuraModDodgePercent,                       // 49 SPELL_AURA_MOD_DODGE_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         // 50 SPELL_AURA_MOD_CRITICAL_HEALING_AMOUNT implemented in Unit::SpellCriticalHealingBonus
    &AuraEffect::HandleAuraModBlockPercent,                       // 51 SPELL_AURA_MOD_BLOCK_PERCENT
    &AuraEffect::HandleAuraModCritPercent,                        // 52 SPELL_AURA_MOD_CRIT_PERCENT
    &AuraEffect::HandlePeriodicLeech,                             // 53 SPELL_AURA_PERIODIC_LEECH
    &AuraEffect::HandleModHitChance,                              // 54 SPELL_AURA_MOD_HIT_CHANCE
    &AuraEffect::HandleModSpellHitChance,                         // 55 SPELL_AURA_MOD_SPELL_HIT_CHANCE
    &AuraEffect::HandleAuraTransform,                             // 56 SPELL_AURA_TRANSFORM
    &AuraEffect::HandleModSpellCritChance,                        // 57 SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    &AuraEffect::HandleAuraModIncreaseSwimSpeed,                  // 58 SPELL_AURA_MOD_INCREASE_SWIM_SPEED
    &AuraEffect::HandleNoImmediateEffect,                         // 59 SPELL_AURA_MOD_DAMAGE_DONE_CREATURE implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &AuraEffect::HandleAuraModPacifyAndSilence,                   // 60 SPELL_AURA_MOD_PACIFY_SILENCE
    &AuraEffect::HandleAuraModScale,                              // 61 SPELL_AURA_MOD_SCALE
    &AuraEffect::HandlePeriodicHealthFunnel,                      // 62 SPELL_AURA_PERIODIC_HEALTH_FUNNEL
    &AuraEffect::HandleUnused,                                    // 63 unused (3.0.8a) old SPELL_AURA_PERIODIC_MANA_FUNNEL
    &AuraEffect::HandlePeriodicManaLeech,                         // 64 SPELL_AURA_PERIODIC_MANA_LEECH
    &AuraEffect::HandleModCastingSpeed,                           // 65 SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK
    &AuraEffect::HandleFeignDeath,                                // 66 SPELL_AURA_FEIGN_DEATH
    &AuraEffect::HandleAuraModDisarm,                             // 67 SPELL_AURA_MOD_DISARM
    &AuraEffect::HandleAuraModStalked,                            // 68 SPELL_AURA_MOD_STALKED
    &AuraEffect::HandleSchoolAbsorb,                              // 69 SPELL_AURA_SCHOOL_ABSORB implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleUnused,                                    // 70 SPELL_AURA_EXTRA_ATTACKS      Useless, used by only one spell that has only visual effect
    &AuraEffect::HandleModSpellCritChanceShool,                   // 71 SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    &AuraEffect::HandleModPowerCostPCT,                           // 72 SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT
    &AuraEffect::HandleModPowerCost,                              // 73 SPELL_AURA_MOD_POWER_COST_SCHOOL
    &AuraEffect::HandleNoImmediateEffect,                         // 74 SPELL_AURA_REFLECT_SPELLS_SCHOOL  implemented in Unit::SpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         // 75 SPELL_AURA_MOD_LANGUAGE
    &AuraEffect::HandleFarSight,                                  // 76 SPELL_AURA_FAR_SIGHT
    &AuraEffect::HandleModMechanicImmunity,                       // 77 SPELL_AURA_MECHANIC_IMMUNITY
    &AuraEffect::HandleAuraMounted,                               // 78 SPELL_AURA_MOUNTED
    &AuraEffect::HandleModDamagePercentDone,                      // 79 SPELL_AURA_MOD_DAMAGE_PERCENT_DONE
    &AuraEffect::HandleModPercentStat,                            // 80 SPELL_AURA_MOD_PERCENT_STAT
    &AuraEffect::HandleNoImmediateEffect,                         // 81 SPELL_AURA_SPLIT_DAMAGE_PCT
    &AuraEffect::HandleWaterBreathing,                            // 82 SPELL_AURA_WATER_BREATHING
    &AuraEffect::HandleModBaseResistance,                         // 83 SPELL_AURA_MOD_BASE_RESISTANCE
    &AuraEffect::HandleModRegen,                                  // 84 SPELL_AURA_MOD_REGEN
    &AuraEffect::HandleModPowerRegen,                             // 85 SPELL_AURA_MOD_POWER_REGEN
    &AuraEffect::HandleChannelDeathItem,                          // 86 SPELL_AURA_CHANNEL_DEATH_ITEM
    &AuraEffect::HandleNoImmediateEffect,                         // 87 SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         // 88 SPELL_AURA_MOD_HEALTH_REGEN_PERCENT
    &AuraEffect::HandlePeriodicDamagePCT,                         // 89 SPELL_AURA_PERIODIC_DAMAGE_PERCENT
    &AuraEffect::HandleUnused,                                    // 90 unused (3.0.8a) old SPELL_AURA_MOD_RESIST_CHANCE
    &AuraEffect::HandleNoImmediateEffect,                         // 91 SPELL_AURA_MOD_DETECT_RANGE implemented in Creature::GetAttackDistance
    &AuraEffect::HandlePreventFleeing,                            // 92 SPELL_AURA_PREVENTS_FLEEING
    &AuraEffect::HandleModUnattackable,                           // 93 SPELL_AURA_MOD_UNATTACKABLE
    &AuraEffect::HandleNoImmediateEffect,                         // 94 SPELL_AURA_INTERRUPT_REGEN implemented in Player::RegenerateAll
    &AuraEffect::HandleAuraGhost,                                 // 95 SPELL_AURA_GHOST
    &AuraEffect::HandleNoImmediateEffect,                         // 96 SPELL_AURA_SPELL_MAGNET implemented in Spell::SelectMagnetTarget
    &AuraEffect::HandleManaShield,                                // 97 SPELL_AURA_MANA_SHIELD implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleAuraModSkill,                              // 98 SPELL_AURA_MOD_SKILL_TALENT
    &AuraEffect::HandleAuraModAttackPower,                        // 99 SPELL_AURA_MOD_ATTACK_POWER
    &AuraEffect::HandleUnused,                                    //100 SPELL_AURA_AURAS_VISIBLE obsolete? all player can see all auras now, but still have spells including GM-spell
    &AuraEffect::HandleModResistancePercent,                      //101 SPELL_AURA_MOD_RESISTANCE_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //102 SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModTotalThreat,                        //103 SPELL_AURA_MOD_TOTAL_THREAT
    &AuraEffect::HandleAuraWaterWalk,                             //104 SPELL_AURA_WATER_WALK
    &AuraEffect::HandleAuraFeatherFall,                           //105 SPELL_AURA_FEATHER_FALL
    &AuraEffect::HandleAuraHover,                                 //106 SPELL_AURA_HOVER
    &AuraEffect::HandleAddModifier,                               //107 SPELL_AURA_ADD_FLAT_MODIFIER
    &AuraEffect::HandleAddModifier,                               //108 SPELL_AURA_ADD_PCT_MODIFIER
    &AuraEffect::HandleAddTargetTrigger,                          //109 SPELL_AURA_ADD_TARGET_TRIGGER
    &AuraEffect::HandleModPowerRegenPCT,                          //110 SPELL_AURA_MOD_POWER_REGEN_PERCENT
    &AuraEffect::HandleNULL,                                      //111 SPELL_AURA_ADD_CASTER_HIT_TRIGGER chance redirect attack to caster
    &AuraEffect::HandleNoImmediateEffect,                         //112 SPELL_AURA_OVERRIDE_CLASS_SCRIPTS
    &AuraEffect::HandleNoImmediateEffect,                         //113 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //114 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //115 SPELL_AURA_MOD_HEALING                 implemented in Unit::SpellBaseHealingBonusForVictim
    &AuraEffect::HandleNoImmediateEffect,                         //116 SPELL_AURA_MOD_REGEN_DURING_COMBAT
    &AuraEffect::HandleNoImmediateEffect,                         //117 SPELL_AURA_MOD_MECHANIC_RESISTANCE     implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //118 SPELL_AURA_MOD_HEALING_PCT             implemented in Unit::SpellHealingBonus
    &AuraEffect::HandleUnused,                                    //119 unused (3.0.8a) old SPELL_AURA_SHARE_PET_TRACKING
    &AuraEffect::HandleAuraUntrackable,                           //120 SPELL_AURA_UNTRACKABLE
    &AuraEffect::HandleAuraEmpathy,                               //121 SPELL_AURA_EMPATHY
    &AuraEffect::HandleModOffhandDamagePercent,                   //122 SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT
    &AuraEffect::HandleModTargetResistance,                       //123 SPELL_AURA_MOD_TARGET_RESISTANCE
    &AuraEffect::HandleAuraModRangedAttackPower,                  //124 SPELL_AURA_MOD_RANGED_ATTACK_POWER
    &AuraEffect::HandleNoImmediateEffect,                         //125 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //126 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //127 SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleModPossessPet,                             //128 SPELL_AURA_MOD_POSSESS_PET
    &AuraEffect::HandleAuraModIncreaseSpeed,                      //129 SPELL_AURA_MOD_SPEED_ALWAYS
    &AuraEffect::HandleAuraModIncreaseMountedSpeed,               //130 SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS
    &AuraEffect::HandleNoImmediateEffect,                         //131 SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModIncreaseEnergyPercent,              //132 SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT
    &AuraEffect::HandleAuraModIncreaseHealthPercent,              //133 SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT
    &AuraEffect::HandleAuraModRegenInterrupt,                     //134 SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
    &AuraEffect::HandleModHealingDone,                            //135 SPELL_AURA_MOD_HEALING_DONE
    &AuraEffect::HandleNoImmediateEffect,                         //136 SPELL_AURA_MOD_HEALING_DONE_PERCENT   implemented in Unit::SpellHealingBonus
    &AuraEffect::HandleModTotalPercentStat,                       //137 SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE
    &AuraEffect::HandleHaste,                                     //138 SPELL_AURA_MOD_HASTE
    &AuraEffect::HandleForceReaction,                             //139 SPELL_AURA_FORCE_REACTION
    &AuraEffect::HandleAuraModRangedHaste,                        //140 SPELL_AURA_MOD_RANGED_HASTE
    &AuraEffect::HandleRangedAmmoHaste,                           //141 SPELL_AURA_MOD_RANGED_AMMO_HASTE
    &AuraEffect::HandleAuraModBaseResistancePCT,                  //142 SPELL_AURA_MOD_BASE_RESISTANCE_PCT
    &AuraEffect::HandleAuraModResistanceExclusive,                //143 SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE
    &AuraEffect::HandleNoImmediateEffect,                         //144 SPELL_AURA_SAFE_FALL                  implemented in WorldSession::HandleMovementOpcodes
    &AuraEffect::HandleAuraModPetTalentsPoints,                   //145 SPELL_AURA_MOD_PET_TALENT_POINTS
    &AuraEffect::HandleNoImmediateEffect,                         //146 SPELL_AURA_ALLOW_TAME_PET_TYPE
    &AuraEffect::HandleModMechanicImmunity,                       //147 SPELL_AURA_MECHANIC_IMMUNITY_MASK
    &AuraEffect::HandleAuraRetainComboPoints,                     //148 SPELL_AURA_RETAIN_COMBO_POINTS
    &AuraEffect::HandleNoImmediateEffect,                         //149 SPELL_AURA_REDUCE_PUSHBACK
    &AuraEffect::HandleShieldBlockValue,                          //150 SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT
    &AuraEffect::HandleAuraTrackStealthed,                        //151 SPELL_AURA_TRACK_STEALTHED
    &AuraEffect::HandleNoImmediateEffect,                         //152 SPELL_AURA_MOD_DETECTED_RANGE implemented in Creature::GetAttackDistance
    &AuraEffect::HandleNoImmediateEffect,                         //153 SPELL_AURA_SPLIT_DAMAGE_FLAT
    &AuraEffect::HandleNoImmediateEffect,                         //154 SPELL_AURA_MOD_STEALTH_LEVEL
    &AuraEffect::HandleNoImmediateEffect,                         //155 SPELL_AURA_MOD_WATER_BREATHING
    &AuraEffect::HandleNoImmediateEffect,                         //156 SPELL_AURA_MOD_REPUTATION_GAIN
    &AuraEffect::HandleNULL,                                      //157 SPELL_AURA_PET_DAMAGE_MULTI
    &AuraEffect::HandleShieldBlockValue,                          //158 SPELL_AURA_MOD_SHIELD_BLOCKVALUE
    &AuraEffect::HandleNoImmediateEffect,                         //159 SPELL_AURA_NO_PVP_CREDIT      only for Honorless Target spell
    &AuraEffect::HandleNoImmediateEffect,                         //160 SPELL_AURA_MOD_AOE_AVOIDANCE                 implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //161 SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT
    &AuraEffect::HandleAuraPowerBurn,                             //162 SPELL_AURA_POWER_BURN_MANA
    &AuraEffect::HandleNoImmediateEffect,                         //163 SPELL_AURA_MOD_CRIT_DAMAGE_BONUS_MELEE
    &AuraEffect::HandleUnused,                                    //164 unused (3.0.8a), only one test spell
    &AuraEffect::HandleNoImmediateEffect,                         //165 SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModAttackPowerPercent,                 //166 SPELL_AURA_MOD_ATTACK_POWER_PCT
    &AuraEffect::HandleAuraModRangedAttackPowerPercent,           //167 SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //168 SPELL_AURA_MOD_DAMAGE_DONE_VERSUS            implemented in Unit::SpellDamageBonus, Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //169 SPELL_AURA_MOD_CRIT_PERCENT_VERSUS           implemented in Unit::DealDamageBySchool, Unit::DoAttackDamage, Unit::SpellCriticalBonus
    &AuraEffect::HandleNULL,                                      //170 SPELL_AURA_DETECT_AMORE       different spells that ignore transformation effects
    &AuraEffect::HandleAuraModIncreaseSpeed,                      //171 SPELL_AURA_MOD_SPEED_NOT_STACK
    &AuraEffect::HandleAuraModIncreaseMountedSpeed,               //172 SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK
    &AuraEffect::HandleUnused,                                    //173 unused (3.0.8a) no spells, old SPELL_AURA_ALLOW_CHAMPION_SPELLS  only for Proclaim Champion spell
    &AuraEffect::HandleModSpellDamagePercentFromStat,             //174 SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT  implemented in Unit::SpellBaseDamageBonus
    &AuraEffect::HandleModSpellHealingPercentFromStat,            //175 SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT implemented in Unit::SpellBaseHealingBonus
    &AuraEffect::HandleSpiritOfRedemption,                        //176 SPELL_AURA_SPIRIT_OF_REDEMPTION   only for Spirit of Redemption spell, die at aura end
    &AuraEffect::HandleNULL,                                      //177 SPELL_AURA_AOE_CHARM
    &AuraEffect::HandleNoImmediateEffect,                         //178 SPELL_AURA_MOD_DEBUFF_RESISTANCE          implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //179 SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE implemented in Unit::SpellCriticalBonus
    &AuraEffect::HandleNoImmediateEffect,                         //180 SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS   implemented in Unit::SpellDamageBonus
    &AuraEffect::HandleUnused,                                    //181 unused (3.0.8a) old SPELL_AURA_MOD_FLAT_SPELL_CRIT_DAMAGE_VERSUS
    &AuraEffect::HandleAuraModResistenceOfStatPercent,            //182 SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT
    &AuraEffect::HandleNULL,                                      //183 SPELL_AURA_MOD_CRITICAL_THREAT only used in 28746
    &AuraEffect::HandleNoImmediateEffect,                         //184 SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE  implemented in Unit::RollMeleeOutcomeAgainst
    &AuraEffect::HandleNoImmediateEffect,                         //185 SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE implemented in Unit::RollMeleeOutcomeAgainst
    &AuraEffect::HandleNoImmediateEffect,                         //186 SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE  implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //187 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE  implemented in Unit::GetUnitCriticalChance
    &AuraEffect::HandleNoImmediateEffect,                         //188 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE implemented in Unit::GetUnitCriticalChance
    &AuraEffect::HandleModRating,                                 //189 SPELL_AURA_MOD_RATING
    &AuraEffect::HandleNULL,                                      //190 SPELL_AURA_MOD_FACTION_REPUTATION_GAIN
    &AuraEffect::HandleAuraModUseNormalSpeed,                     //191 SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED
    &AuraEffect::HandleModMeleeRangedSpeedPct,                    //192 SPELL_AURA_HASTE_MELEE
    &AuraEffect::HandleModCombatSpeedPct,                         //193 SPELL_AURA_MELEE_SLOW (in fact combat (any type attack) speed pct)
    &AuraEffect::HandleNULL,                                      //194 SPELL_AURA_MOD_TARGET_ABSORB_SCHOOL implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleNoImmediateEffect,                         //195 SPELL_AURA_MOD_TARGET_ABILITY_ABSORB_SCHOOL implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleNULL,                                      //196 SPELL_AURA_MOD_COOLDOWN
    &AuraEffect::HandleNoImmediateEffect,                         //197 SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE implemented in Unit::SpellCriticalBonus Unit::GetUnitCriticalChance
    &AuraEffect::HandleUnused,                                    //198 unused (3.0.8a) old SPELL_AURA_MOD_ALL_WEAPON_SKILLS
    &AuraEffect::HandleNoImmediateEffect,                         //199 SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT  implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //200 SPELL_AURA_MOD_XP_PCT implemented in Player::GiveXP
    &AuraEffect::HandleAuraAllowFlight,                           //201 SPELL_AURA_FLY                             this aura enable flight mode...
    &AuraEffect::HandleNoImmediateEffect,                         //202 SPELL_AURA_CANNOT_BE_DODGED                implemented in Unit::RollPhysicalOutcomeAgainst
    &AuraEffect::HandleNoImmediateEffect,                         //203 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE  implemented in Unit::CalculateMeleeDamage and Unit::CalculateSpellDamage
    &AuraEffect::HandleNoImmediateEffect,                         //204 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE implemented in Unit::CalculateMeleeDamage and Unit::CalculateSpellDamage
    &AuraEffect::HandleNULL,                                      //205 vulnerable to school dmg?
    &AuraEffect::HandleNULL,                                      //206 SPELL_AURA_MOD_SPEED_MOUNTED
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //207 SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //208 SPELL_AURA_MOD_SPEED_FLIGHT, used only in spell: Flight Form (Passive)
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //209 SPELL_AURA_MOD_FLIGHT_SPEED_ALWAYS
    &AuraEffect::HandleNULL,                                      //210 Commentator's Command
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //211 SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK
    &AuraEffect::HandleAuraModRangedAttackPowerOfStatPercent,     //212 SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         //213 SPELL_AURA_MOD_RAGE_FROM_DAMAGE_DEALT implemented in Player::RewardRage
    &AuraEffect::HandleNULL,                                      //214 Tamed Pet Passive
    &AuraEffect::HandleArenaPreparation,                          //215 SPELL_AURA_ARENA_PREPARATION
    &AuraEffect::HandleModCastingSpeed,                           //216 SPELL_AURA_HASTE_SPELLS
    &AuraEffect::HandleUnused,                                    //217 unused (3.0.8a)
    &AuraEffect::HandleAuraModRangedHaste,                        //218 SPELL_AURA_HASTE_RANGED
    &AuraEffect::HandleModManaRegen,                              //219 SPELL_AURA_MOD_MANA_REGEN_FROM_STAT
    &AuraEffect::HandleModRatingFromStat,                         //220 SPELL_AURA_MOD_RATING_FROM_STAT
    &AuraEffect::HandleNULL,                                      //221 ignored
    &AuraEffect::HandleUnused,                                    //222 unused (3.0.8a) only for spell 44586 that not used in real spell cast
    &AuraEffect::HandleNoImmediateEffect,                         //223 SPELL_AURA_RAID_PROC_FROM_CHARGE
    &AuraEffect::HandleUnused,                                    //224 unused (3.0.8a)
    &AuraEffect::HandleNoImmediateEffect,                         //225 SPELL_AURA_RAID_PROC_FROM_CHARGE_WITH_VALUE
    &AuraEffect::HandleAuraPeriodicDummy,                         //226 SPELL_AURA_PERIODIC_DUMMY
    &AuraEffect::HandlePeriodicTriggerSpellWithValue,             //227 SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE
    &AuraEffect::HandleNoImmediateEffect,                         //228 stealth detection
    &AuraEffect::HandleNULL,                                      //229 SPELL_AURA_MOD_AOE_DAMAGE_AVOIDANCE
    &AuraEffect::HandleAuraModIncreaseHealth,                     //230 SPELL_AURA_MOD_INCREASE_HEALTH_2
    &AuraEffect::HandleNoImmediateEffect,                         //231 SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE
    &AuraEffect::HandleNoImmediateEffect,                         //232 SPELL_AURA_MECHANIC_DURATION_MOD           implement in Unit::CalculateSpellDuration
    &AuraEffect::HandleNULL,                                      //233 set model id to the one of the creature with id GetMiscValue()
    &AuraEffect::HandleNoImmediateEffect,                         //234 SPELL_AURA_MECHANIC_DURATION_MOD_NOT_STACK implement in Unit::CalculateSpellDuration
    &AuraEffect::HandleAuraModDispelResist,                       //235 SPELL_AURA_MOD_DISPEL_RESIST               implement in Unit::MagicSpellHitResult
    &AuraEffect::HandleAuraControlVehicle,                        //236 SPELL_AURA_CONTROL_VEHICLE
    &AuraEffect::HandleModSpellDamagePercentFromAttackPower,      //237 SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER  implemented in Unit::SpellBaseDamageBonus
    &AuraEffect::HandleModSpellHealingPercentFromAttackPower,     //238 SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER implemented in Unit::SpellBaseHealingBonus
    &AuraEffect::HandleAuraModScale,                              //239 SPELL_AURA_MOD_SCALE_2 only in Noggenfogger Elixir (16595) before 2.3.0 aura 61
    &AuraEffect::HandleAuraModExpertise,                          //240 SPELL_AURA_MOD_EXPERTISE
    &AuraEffect::HandleForceMoveForward,                          //241 Forces the player to move forward
    &AuraEffect::HandleUnused,                                    //242 SPELL_AURA_MOD_SPELL_DAMAGE_FROM_HEALING
    &AuraEffect::HandleNULL,                                      //243 faction reaction override spells
    &AuraEffect::HandleComprehendLanguage,                        //244 Comprehend language
    &AuraEffect::HandleNoImmediateEffect,                         //245 SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL
    &AuraEffect::HandleNoImmediateEffect,                         //246 SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL_NOT_STACK implemented in Spell::EffectApplyAura
    &AuraEffect::HandleNULL,                                      //247 target to become a clone of the caster
    &AuraEffect::HandleNoImmediateEffect,                         //248 SPELL_AURA_MOD_COMBAT_RESULT_CHANCE         implemented in Unit::RollMeleeOutcomeAgainst
    &AuraEffect::HandleAuraConvertRune,                           //249 SPELL_AURA_CONVERT_RUNE
    &AuraEffect::HandleAuraModIncreaseHealth,                     //250 SPELL_AURA_MOD_INCREASE_HEALTH_2
    &AuraEffect::HandleNoImmediateEffect,                         //251 SPELL_AURA_MOD_ENEMY_DODGE
    &AuraEffect::HandleNULL,                                      //252 haste all?
    &AuraEffect::HandleNoImmediateEffect,                         //253 SPELL_AURA_MOD_BLOCK_CRIT_CHANCE  implemented in Unit::isBlockCritical
    &AuraEffect::HandleAuraModDisarm,                             //254 SPELL_AURA_MOD_DISARM_OFFHAND
    &AuraEffect::HandleNoImmediateEffect,                         //255 SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT    implemented in Unit::SpellDamageBonus
    &AuraEffect::HandleNoReagentUseAura,                          //256 SPELL_AURA_NO_REAGENT_USE Use SpellClassMask for spell select
    &AuraEffect::HandleNULL,                                      //257 SPELL_AURA_MOD_TARGET_RESIST_BY_SPELL_CLASS Use SpellClassMask for spell select
    &AuraEffect::HandleNULL,                                      //258 SPELL_AURA_MOD_SPELL_VISUAL
    &AuraEffect::HandleNoImmediateEffect,                         //259 SPELL_AURA_MOD_HOT_PCT implemented in Unit::SpellHealingBonus
    &AuraEffect::HandleNoImmediateEffect,                         //260 SPELL_AURA_SCREEN_EFFECT (miscvalue = id in ScreenEffect.dbc) not required any code
    &AuraEffect::HandlePhase,                                     //261 SPELL_AURA_PHASE undetactable invisibility?     implemented in Unit::isVisibleForOrDetect
    &AuraEffect::HandleNoImmediateEffect,                         //262 SPELL_AURA_ABILITY_IGNORE_AURASTATE implemented in spell::cancast
    &AuraEffect::HandleAuraAllowOnlyAbility,                      //263 SPELL_AURA_ALLOW_ONLY_ABILITY player can use only abilities set in SpellClassMask
    &AuraEffect::HandleUnused,                                    //264 unused (3.0.8a)
    &AuraEffect::HandleUnused,                                    //265 unused (3.0.8a)
    &AuraEffect::HandleUnused,                                    //266 unused (3.0.8a)
    &AuraEffect::HandleNoImmediateEffect,                         //267 SPELL_AURA_MOD_IMMUNE_AURA_APPLY_SCHOOL         implemented in Unit::IsImmunedToSpellEffect
    &AuraEffect::HandleAuraModAttackPowerOfStatPercent,           //268 SPELL_AURA_MOD_ATTACK_POWER_OF_STAT_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         //269 SPELL_AURA_MOD_IGNORE_TARGET_RESIST implemented in Unit::CalcAbsorbResist and CalcArmorReducedDamage
    &AuraEffect::HandleNoImmediateEffect,                         //270 SPELL_AURA_MOD_ABILITY_IGNORE_TARGET_RESIST implemented in Unit::CalcAbsorbResist and CalcArmorReducedDamage
    &AuraEffect::HandleNoImmediateEffect,                         //271 SPELL_AURA_MOD_DAMAGE_FROM_CASTER    implemented in Unit::SpellDamageBonus
    &AuraEffect::HandleNULL,                                      //272 reduce spell cast time?
    &AuraEffect::HandleUnused,                                    //273 clientside
    &AuraEffect::HandleNoImmediateEffect,                         //274 SPELL_AURA_CONSUME_NO_AMMO implemented in spell::CalculateDamageDoneForAllTargets
    &AuraEffect::HandleNoImmediateEffect,                         //275 SPELL_AURA_MOD_IGNORE_SHAPESHIFT Use SpellClassMask for spell select
    &AuraEffect::HandleNULL,                                      //276 mod damage % mechanic?
    &AuraEffect::HandleNoImmediateEffect,                         //277 SPELL_AURA_MOD_ABILITY_AFFECTED_TARGETS implemented in spell::settargetmap
    &AuraEffect::HandleAuraModDisarm,                             //278 SPELL_AURA_MOD_DISARM_RANGED disarm ranged weapon
    &AuraEffect::HandleNULL,                                      //279 visual effects? 58836 and 57507
    &AuraEffect::HandleNoImmediateEffect,                         //280 SPELL_AURA_MOD_WEAPONTYPE_IGNORE_TARGET_RESISTANCE
    &AuraEffect::HandleNoImmediateEffect,                         //281 SPELL_AURA_MOD_HONOR_GAIN_PCT implemented in Player::RewardHonor
    &AuraEffect::HandleAuraIncreaseBaseHealthPercent,             //282 SPELL_AURA_INCREASE_BASE_HEALTH_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         //283 SPELL_AURA_MOD_HEALING_RECEIVED       implemented in Unit::SpellHealingBonus
    &AuraEffect::HandleUnused,                                    //284 not used by any spells (3.08a)
    &AuraEffect::HandleUnused,                                    //285 not used by any spells (3.08a)
    &AuraEffect::HandleUnused,                                    //286 not used by any spells (3.08a)
    &AuraEffect::HandleNoImmediateEffect,                         //287 SPELL_AURA_DEFLECT_SPELLS             implemented in Unit::MagicSpellHitResult and Unit::MeleeSpellHitResult
    &AuraEffect::HandleUnused,                                    //288 not used by any spells (3.09) except 1 test spell.
};

Aura::Aura(SpellEntry const* spellproto, uint32 effMask, int32 *currentBasePoints, Unit *target, Unit *caster, Item* castItem) :
m_caster_guid(0), m_castItemGuid(castItem?castItem->GetGUID():0), m_target(target),
m_timeCla(1000), m_removeMode(AURA_REMOVE_BY_DEFAULT), m_AuraDRGroup(DIMINISHING_NONE),
m_auraSlot(MAX_AURAS), m_auraLevel(1), m_procCharges(0), m_stackAmount(1),m_auraStateMask(0), m_updated(false), m_isRemoved(false)
{
    assert(target);

    assert(spellproto && spellproto == sSpellStore.LookupEntry( spellproto->Id ) && "`info` must be pointer to sSpellStore element");

    m_auraFlags = effMask;

    m_spellProto = spellproto;

    m_isPassive = IsPassiveSpell(GetId());

    m_auraStateMask = 0;

    m_isSingleTargetAura = IsSingleTargetSpell(m_spellProto);

    m_applyTime = time(NULL);

    if(!caster)
    {
        m_caster_guid = target->GetGUID();
        //damage = m_currentBasePoints+1;                     // stored value-1
        m_maxduration = target->CalcSpellDuration(m_spellProto);
    }
    else
    {
        m_caster_guid = caster->GetGUID();

        //damage        = caster->CalculateSpellDamage(m_spellProto,m_effIndex,m_currentBasePoints,target);
        m_maxduration = caster->CalcSpellDuration(m_spellProto);
    }

    if(m_maxduration == -1 || m_isPassive && m_spellProto->DurationIndex == 0)
        m_permanent = true;
    else
        m_permanent = false;

    Player* modOwner = caster ? caster->GetSpellModOwner() : NULL;

    if(!m_permanent && modOwner)
    {
        modOwner->ApplySpellMod(GetId(), SPELLMOD_DURATION, m_maxduration);
        // Get zero duration aura after - need set m_maxduration > 0 for apply/remove aura work
        if (m_maxduration<=0)
            m_maxduration = 1;
    }

    m_duration = m_maxduration;

    m_isDeathPersist = IsDeathPersistentSpell(m_spellProto);

    m_procCharges = m_spellProto->procCharges;
    if(modOwner)
        modOwner->ApplySpellMod(GetId(), SPELLMOD_CHARGES, m_procCharges);

    m_isRemovedOnShapeLost = (m_caster_guid==m_target->GetGUID() &&
                              m_spellProto->Stances &&
                            !(m_spellProto->AttributesEx2 & SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT) &&
                            !(m_spellProto->Attributes & SPELL_ATTR_NOT_SHAPESHIFT));

    for (uint8 i=0 ;i<MAX_SPELL_EFFECTS;++i)
    {
        if (m_auraFlags & (uint8(1) << i))
        {
            if (&currentBasePoints[0])
                m_partAuras[i]=CreateAuraEffect(this, i,&currentBasePoints[0]+i, caster);
            else
                m_partAuras[i]=CreateAuraEffect(this, i, NULL , caster);
            // correct flags if aura couldn't be created
            if (!m_partAuras[i])
                m_auraFlags &= uint8(~(1<< i));
        }
        else
        {
            m_partAuras[i]=NULL;
        }
    }

    // Aura is positive when it is casted by friend and at least one aura is positive
    // or when it is casted by enemy and  at least one aura is negative
    bool swap=false;
    if (!caster || caster==target)
        m_positive=IsPositiveSpell(m_spellProto->Id);
    else
    {
        m_positive = !caster->IsHostileTo(m_target);
        for (uint8 i=0;i<MAX_SPELL_EFFECTS;++i)
        {
            if (!(1<<i & GetEffectMask()))
                continue;
            if (m_positive == IsPositiveEffect(GetId(), i))
            {
                swap = true;
                break;
            }
        }
        if (!swap)
            m_positive=!m_positive;
    }
}

Aura::~Aura()
{
    // free part auras memory
    for (uint8 i=0 ; i<MAX_SPELL_EFFECTS;++i)
        if (m_partAuras[i])
            delete m_partAuras[i];
}

AuraEffect::AuraEffect(Aura * parentAura, uint8 effIndex, int32 * currentBasePoints , Unit * caster, Item* castItem) :
m_parentAura(parentAura), m_spellmod(NULL), m_periodicTimer(0), m_isPeriodic(false), m_isAreaAura(false), m_isPersistent(false),
m_target(parentAura->GetTarget())
{
    m_spellProto = parentAura->GetSpellProto();
    m_effIndex = effIndex;
    m_auraName = AuraType(m_spellProto->EffectApplyAuraName[m_effIndex]);

    if(currentBasePoints)
        m_currentBasePoints = *currentBasePoints;
    else
        m_currentBasePoints = m_spellProto->EffectBasePoints[m_effIndex];

    if(caster)
        m_amount = caster->CalculateSpellDamage(m_spellProto, m_effIndex, m_currentBasePoints, m_target);
    else
        m_amount = m_currentBasePoints + 1;

    if (!m_amount && castItem && castItem->GetItemSuffixFactor())
    {
        ItemRandomSuffixEntry const *item_rand_suffix = sItemRandomSuffixStore.LookupEntry(abs(castItem->GetItemRandomPropertyId()));
        if(item_rand_suffix)
        {
            for (int k=0; k<3; k++)
            {
                SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(item_rand_suffix->enchant_id[k]);
                if(pEnchant)
                {
                    for (int t=0; t<3; t++)
                        if(pEnchant->spellid[t] == m_spellProto->Id)
                    {
                        m_amount = uint32((item_rand_suffix->prefix[k]*castItem->GetItemSuffixFactor()) / 10000 );
                        break;
                    }
                }

                if(m_amount)
                    break;
            }
        }
    }

    Player* modOwner = caster ? caster->GetSpellModOwner() : NULL;
    m_amplitude = m_spellProto->EffectAmplitude[m_effIndex];

    //apply casting time mods for channeled spells
    if (modOwner && m_amplitude && IsChanneledSpell(m_spellProto))
        modOwner->ModSpellCastTime(m_spellProto, m_amplitude);

    // Apply periodic time mod
    if(modOwner && m_amplitude)
        modOwner->ApplySpellMod(GetId(), SPELLMOD_ACTIVATION_TIME, m_amplitude);

    // Start periodic on next tick or at aura apply
    if (!(m_spellProto->AttributesEx5 & SPELL_ATTR_EX5_START_PERIODIC_AT_APPLY))
        m_periodicTimer += m_amplitude;
}

AreaAuraEffect::AreaAuraEffect(Aura * parentAura, uint32 effIndex, int32 * currentBasePoints, Unit * caster, Item * castItem)
: AuraEffect(parentAura, effIndex, currentBasePoints, caster, castItem)
{
    m_removeTime = FRIENDLY_AA_REMOVE_TIME;
    m_isAreaAura = true;

    Unit* caster_ptr = caster ? caster : m_target;

    if (m_spellProto->Effect[effIndex] == SPELL_EFFECT_APPLY_AREA_AURA_ENEMY)
        m_radius = GetSpellRadiusForHostile(sSpellRadiusStore.LookupEntry(GetSpellProto()->EffectRadiusIndex[m_effIndex]));
    else
        m_radius = GetSpellRadiusForFriend(sSpellRadiusStore.LookupEntry(GetSpellProto()->EffectRadiusIndex[m_effIndex]));

    if(Player* modOwner = caster_ptr->GetSpellModOwner())
        modOwner->ApplySpellMod(GetId(), SPELLMOD_RADIUS, m_radius);

    switch(m_spellProto->Effect[effIndex])
    {
        case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
            m_areaAuraType = AREA_AURA_PARTY;
            if(m_target->GetTypeId() == TYPEID_UNIT && ((Creature*)m_target)->isTotem())
                m_auraName = SPELL_AURA_NONE;
            break;
        case SPELL_EFFECT_APPLY_AREA_AURA_RAID:
            m_areaAuraType = AREA_AURA_RAID;
            if(m_target->GetTypeId() == TYPEID_UNIT && ((Creature*)m_target)->isTotem())
                m_auraName = SPELL_AURA_NONE;
            break;
        case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
            m_areaAuraType = AREA_AURA_FRIEND;
            break;
        case SPELL_EFFECT_APPLY_AREA_AURA_ENEMY:
            m_areaAuraType = AREA_AURA_ENEMY;
            if(m_target == caster_ptr)
                m_auraName = SPELL_AURA_NONE;    // Do not do any effect on self
            break;
        case SPELL_EFFECT_APPLY_AREA_AURA_PET:
            m_areaAuraType = AREA_AURA_PET;
            break;
        case SPELL_EFFECT_APPLY_AREA_AURA_OWNER:
            m_areaAuraType = AREA_AURA_OWNER;
            if(m_target == caster_ptr)
                m_auraName = SPELL_AURA_NONE;
            break;
        default:
            sLog.outError("Wrong spell effect in AreaAura constructor");
            ASSERT(false);
            break;
    }
}

AreaAuraEffect::~AreaAuraEffect()
{
}

PersistentAreaAuraEffect::PersistentAreaAuraEffect(Aura * parentAura, uint32 effIndex, int32 * currentBasePoints, Unit * caster,Item * castItem)
: AuraEffect(parentAura, effIndex, currentBasePoints, caster, castItem)
{
    m_isPersistent = true;
}

PersistentAreaAuraEffect::~PersistentAreaAuraEffect()
{
}

AuraEffect* CreateAuraEffect(Aura * parentAura, uint32 effIndex, int32 *currentBasePoints, Unit * caster, Item * castItem)
{
    assert (parentAura);
    if (IsAreaAuraEffect(parentAura->GetSpellProto()->Effect[effIndex]))
        return new AreaAuraEffect(parentAura, effIndex, currentBasePoints, caster, castItem);
    else if (parentAura->GetSpellProto()->Effect[effIndex] == SPELL_EFFECT_APPLY_AURA)
        return new AuraEffect(parentAura, effIndex, currentBasePoints, caster, castItem);
    else if (parentAura->GetSpellProto()->Effect[effIndex] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
        return new PersistentAreaAuraEffect(parentAura, effIndex, currentBasePoints, caster, castItem);
    else return NULL;
}

Unit* Aura::GetCaster() const
{
    if(m_caster_guid==m_target->GetGUID())
        return m_target;

    //return ObjectAccessor::GetUnit(*m_target,m_caster_guid);
    //must return caster even if it's in another grid/map
    Unit *unit = ObjectAccessor::GetObjectInWorld(m_caster_guid, (Unit*)NULL);
    return unit && unit->IsInWorld() ? unit : NULL;
}

void Aura::Update(uint32 diff)
{
    if (m_duration > 0)
    {
        m_duration -= diff;
        if (m_duration < 0)
            m_duration = 0;
        m_timeCla -= diff;

        // all spells with manaPerSecond/manaPerSecondPerLevel have aura in effect 0
        if(m_timeCla <= 0)
        {
            if(Unit* caster = GetCaster())
            {
                Powers powertype = Powers(m_spellProto->powerType);
                int32 manaPerSecond = m_spellProto->manaPerSecond + m_spellProto->manaPerSecondPerLevel * caster->getLevel();
                m_timeCla = 1000;
                if (manaPerSecond)
                {
                    if(powertype==POWER_HEALTH)
                    {
                        if (caster->GetHealth()>manaPerSecond)
                            caster->ModifyHealth(-manaPerSecond);
                        else
                            m_target->RemoveAura(this);
                    }
                    else if (caster->GetPower(powertype)>=manaPerSecond)
                        caster->ModifyPower(powertype,-manaPerSecond);
                    else
                        m_target->RemoveAura(this);
                }
            }
        }
    }

    // Channeled aura required check distance from caster except in possessed cases
    if(IsChanneledSpell(m_spellProto) && m_caster_guid != m_target->GetGUID() && !m_target->isPossessed())
    {
        Unit* caster = GetCaster();
        if(!caster)
        {
            m_target->RemoveAura(this);
            return;
        }
        // Get spell range
        float radius=-1.0f;
        SpellModOp mod;
        for (uint8 i=0;i<3;++i)
        {
            if (HasEffect(i) && m_spellProto->EffectRadiusIndex[i])
            {
                radius = caster->GetSpellRadiusForTarget(m_target, sSpellRadiusStore.LookupEntry(m_spellProto->EffectRadiusIndex[i]));
                mod = SPELLMOD_RADIUS;
                break;
            }
        }
        if (radius<0)
        {
            radius = caster->GetSpellMaxRangeForTarget(m_target, sSpellRangeStore.LookupEntry(m_spellProto->rangeIndex)) ;
            mod = SPELLMOD_RANGE;
        }

        if(Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetId(), mod, radius,NULL);

        if(!caster->IsWithinDistInMap(m_target,radius))
        {
            m_target->RemoveAura(this);
            return;
        }
    }

    for (uint8 i = 0; i<MAX_SPELL_EFFECTS;++i)
        if (m_partAuras[i])
        {
            if (m_partAuras[i]->IsAreaAura())
                ((AreaAuraEffect *) m_partAuras[i])->Update(diff);
            else if(m_partAuras[i]->IsPersistent())
                ((PersistentAreaAuraEffect *) m_partAuras[i])->Update(diff);
            else
                m_partAuras[i]->Update(diff);
        }
}

void AuraEffect::Update(uint32 diff)
{
    if (m_isPeriodic && (GetParentAura()->GetAuraDuration() >=0 || GetParentAura()->IsPassive() || GetParentAura()->IsPermanent()))
    {
        m_periodicTimer -= diff;
        if(m_periodicTimer <= 0) // tick also at m_periodicTimer==0 to prevent lost last tick in case max m_duration == (max m_periodicTimer)*N
        {
            // update before applying (aura can be removed in TriggerSpell or PeriodicTick calls)
            m_periodicTimer += m_amplitude;

            if(!m_target->hasUnitState(UNIT_STAT_ISOLATED))
                PeriodicTick();
        }
    }
}

void AreaAuraEffect::Update(uint32 diff)
{
    // update for the caster of the aura
    if(GetCasterGUID() == m_target->GetGUID())
    {
        Unit* caster = m_target;

        if( !caster->hasUnitState(UNIT_STAT_ISOLATED) )
        {
            std::list<Unit *> targets;

            switch(m_areaAuraType)
            {
                case AREA_AURA_PARTY:
                    caster->GetPartyMember(targets, m_radius);
                    break;
                case AREA_AURA_RAID:
                    caster->GetRaidMember(targets, m_radius);
                    break;
                case AREA_AURA_FRIEND:
                {
                    Trinity::AnyFriendlyUnitInObjectRangeCheck u_check(caster, caster, m_radius);
                    Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(caster, targets, u_check);
                    caster->VisitNearbyObject(m_radius, searcher);
                    break;
                }
                case AREA_AURA_ENEMY:
                {
                    Trinity::AnyAoETargetUnitInObjectRangeCheck u_check(caster, caster, m_radius); // No GetCharmer in searcher
                    Trinity::UnitListSearcher<Trinity::AnyAoETargetUnitInObjectRangeCheck> searcher(caster, targets, u_check);
                    caster->VisitNearbyObject(m_radius, searcher);
                    break;
                }
                case AREA_AURA_OWNER:
                case AREA_AURA_PET:
                {
                    if(Unit *owner = caster->GetCharmerOrOwner())
                        targets.push_back(owner);
                    break;
                }
            }

            for(std::list<Unit *>::iterator tIter = targets.begin(); tIter != targets.end(); tIter++)
            {
                if((*tIter)->HasAuraEffect(GetId(), GetEffIndex(), caster->GetGUID()))
                    continue;

                if(SpellEntry const *actualSpellInfo = spellmgr.SelectAuraRankForPlayerLevel(GetSpellProto(), (*tIter)->getLevel()))
                {
                    //int32 actualBasePoints = m_currentBasePoints;
                    // recalculate basepoints for lower rank (all AreaAura spell not use custom basepoints?)
                    //if(actualSpellInfo != GetSpellProto())
                    //    actualBasePoints = actualSpellInfo->EffectBasePoints[m_effIndex];
                    Aura * aur = (*tIter)->AddAuraEffect(actualSpellInfo->Id, GetEffIndex(), caster);

                    if(m_areaAuraType == AREA_AURA_ENEMY)
                        caster->CombatStart(*tIter);
                }
            }
        }
        AuraEffect::Update(diff);
    }
    else                                                    // aura at non-caster
    {
        Unit * tmp_target = m_target;
        Unit* caster = GetCaster();
        uint32 tmp_spellId = GetId();
        uint32 tmp_effIndex = GetEffIndex();
        uint64 tmp_guid = GetCasterGUID();

        // WARNING: the aura may get deleted during the update
        // DO NOT access its members after update!
        AuraEffect::Update(diff);

        // remove aura if out-of-range from caster (after teleport for example)
        // or caster is isolated or caster no longer has the aura
        // or caster is (no longer) friendly
        bool needFriendly = (m_areaAuraType == AREA_AURA_ENEMY ? false : true);
        if( !caster || caster->hasUnitState(UNIT_STAT_ISOLATED) ||
            !caster->HasAuraEffect(tmp_spellId, tmp_effIndex)         ||
            caster->IsFriendlyTo(tmp_target) != needFriendly
           )
        {
            m_target->RemoveAura(GetParentAura());
        }
        else if (!caster->IsWithinDistInMap(tmp_target, m_radius))
        {
            if (needFriendly)
            {
                m_removeTime -= diff;
                if (m_removeTime < 0)
                    m_target->RemoveAura(GetParentAura());
            }
            else
                m_target->RemoveAura(GetParentAura());
        }
        else
        {
            // Reset aura remove timer
            m_removeTime = FRIENDLY_AA_REMOVE_TIME;
            if( m_areaAuraType == AREA_AURA_PARTY)         // check if in same sub group
            {
                if(!tmp_target->IsInPartyWith(caster))
                    m_target->RemoveAura(GetParentAura());
            }
            else if( m_areaAuraType == AREA_AURA_RAID)
            {
                if(!tmp_target->IsInRaidWith(caster))
                    m_target->RemoveAura(GetParentAura());
            }
            else if( m_areaAuraType == AREA_AURA_PET || m_areaAuraType == AREA_AURA_OWNER )
            {
                if( tmp_target->GetGUID() != caster->GetCharmerOrOwnerGUID() )
                    m_target->RemoveAura(GetParentAura());
            }
        }
    }
}

void PersistentAreaAuraEffect::Update(uint32 diff)
{
    bool remove = false;

    // remove the aura if its caster or the dynamic object causing it was removed
    // or if the target moves too far from the dynamic object
    Unit *caster = GetCaster();
    if (caster)
    {
        DynamicObject *dynObj = caster->GetDynObject(GetId(), GetEffIndex());
        if (dynObj)
        {
            if (!m_target->IsWithinDistInMap(dynObj, dynObj->GetRadius()))
                remove = true;
        }
        else
            remove = true;
    }
    else
        remove = true;

    if(remove)
        m_target->RemoveAura(GetParentAura());

    AuraEffect::Update(diff);
}

void AuraEffect::ApplyModifier(bool apply, bool Real)
{
    if (GetParentAura()->IsRemoved())
        return;

    if(m_auraName<TOTAL_AURAS)
        (*this.*AuraHandler [m_auraName])(apply,Real);
}

void AuraEffect::CleanupTriggeredSpells()
{
    uint32 tSpellId = m_spellProto->EffectTriggerSpell[GetEffIndex()];
    if(!tSpellId)
        return;

    SpellEntry const* tProto = sSpellStore.LookupEntry(tSpellId);
    if(!tProto)
        return;

    if(GetSpellDuration(tProto) != -1)
        return;

    // needed for spell 43680, maybe others
    // TODO: is there a spell flag, which can solve this in a more sophisticated way?
    if(m_spellProto->EffectApplyAuraName[GetEffIndex()] == SPELL_AURA_PERIODIC_TRIGGER_SPELL &&
            GetSpellDuration(m_spellProto) == m_spellProto->EffectAmplitude[GetEffIndex()])
        return;

    m_target->RemoveAurasDueToSpell(tSpellId, GetCasterGUID());
}

void Aura::ApplyAllModifiers(bool apply, bool Real)
{
    for (uint8 i = 0; i<MAX_SPELL_EFFECTS;++i)
        if (m_partAuras[i])
            m_partAuras[i]->ApplyModifier(apply, Real);
}

void Aura::SendAuraUpdate()
{
    if (m_auraSlot>=MAX_AURAS)
        return;
    WorldPacket data(SMSG_AURA_UPDATE);

    data.append(m_target->GetPackGUID());
    data << uint8(m_auraSlot);

    if(!m_target->GetVisibleAura(m_auraSlot))
    {
        data << uint32(0);
        sLog.outDebug("Aura %u removed slot %u",GetId(), m_auraSlot);
        m_target->SendMessageToSet(&data, true);
        return;
    }

    data << uint32(GetId());
    data << uint8(m_auraFlags);
    data << uint8(m_auraLevel);
    data << uint8(m_stackAmount ? m_stackAmount : m_procCharges);

    if(!(m_auraFlags & AFLAG_CASTER))
    {
        if (Unit * caster = GetCaster())
            data.append(caster->GetPackGUID());
        else
            data << uint8(0);
    }

    if(m_auraFlags & AFLAG_DURATION)
    {
        data << uint32(m_maxduration);
        data << uint32(m_duration);
    }

    m_target->SendMessageToSet(&data, true);
}

void Aura::_AddAura()
{
    if (!GetId())
        return;
    if(!m_target)
        return;

    Unit* caster = GetCaster();

    // set infinity cooldown state for spells
    if(caster && caster->GetTypeId() == TYPEID_PLAYER)
    {
        if (m_spellProto->Attributes & SPELL_ATTR_DISABLED_WHILE_ACTIVE)
        {
            Item* castItem = m_castItemGuid ? ((Player*)caster)->GetItemByGuid(m_castItemGuid) : NULL;
            ((Player*)caster)->AddSpellAndCategoryCooldowns(m_spellProto,castItem ? castItem->GetEntry() : 0, NULL,true);
        }
    }

    // passive auras (except totem auras) do not get placed in the slots
    // area auras with SPELL_AURA_NONE are not shown on target
    if((m_spellProto->Attributes & 0x80 && GetTalentSpellPos(GetId())) 
        || !m_isPassive
        || (caster && caster->GetTypeId() == TYPEID_UNIT && ((Creature*)caster)->isTotem())
        || (IsAuraType(SPELL_AURA_ABILITY_IGNORE_AURASTATE))
             && (!IsAreaAura()
            || m_target!=caster ||
            (m_spellProto->Effect[0]!=SPELL_EFFECT_APPLY_AREA_AURA_ENEMY
            && m_spellProto->Effect[1]!=SPELL_EFFECT_APPLY_AREA_AURA_ENEMY
            && m_spellProto->Effect[2]!=SPELL_EFFECT_APPLY_AREA_AURA_ENEMY)))
    {
        // Try find slot for aura
        uint8 slot = MAX_AURAS;
        // Lookup for auras already applied from spell
        if (Aura * foundAura = m_target->GetAura(GetId(), GetCasterGUID()))
        {
            // allow use single slot only by auras from same caster
            slot = foundAura->GetAuraSlot();
        }
        else
        {
            Unit::VisibleAuraMap const * visibleAuras= m_target->GetVisibleAuras();
            // lookup for free slots in units visibleAuras
            Unit::VisibleAuraMap::const_iterator itr = visibleAuras->find(0);
            for(uint32 freeSlot = 0; freeSlot < MAX_AURAS; ++itr , ++freeSlot)
            {
                if(itr == visibleAuras->end() || itr->first != freeSlot)
                {
                    slot = freeSlot;
                    break;
                }
            }
        }

        // Register Visible Aura
        if(slot < MAX_AURAS)
        {
            m_auraFlags |= (IsPositive() ? AFLAG_POSITIVE : AFLAG_NEGATIVE) |
                (GetCasterGUID() == m_target->GetGUID() ? AFLAG_CASTER : AFLAG_NONE) |
                (GetAuraMaxDuration() > 0 ? AFLAG_DURATION : AFLAG_NONE);
            m_auraLevel = (caster ? caster->getLevel() : sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL));
            SetAuraSlot( slot );
            m_target->SetVisibleAura(slot, this);
            m_target->UpdateAuraForGroup(slot);
            SendAuraUpdate();
            sLog.outDebug("Aura: %u Effect: %d put to unit visible auras slot: %u",GetId(), GetEffectMask(), slot);
        }
        else
            sLog.outDebug("Aura: %u Effect: %d could not find empty unit visible slot",GetId(), GetEffectMask());
    }

    // Sitdown on apply aura req seated
    if (m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED && !m_target->IsSitState())
        m_target->SetStandState(UNIT_STAND_STATE_SIT);

    // register aura diminishing on apply
    if (getDiminishGroup() != DIMINISHING_NONE )
        m_target->ApplyDiminishingAura(getDiminishGroup(),true);

    // Apply linked auras (On first aura apply)
    uint32 id = GetId();
    if(spellmgr.GetSpellCustomAttr(id) & SPELL_ATTR_CU_LINK_AURA)
    {
        if(const std::vector<int32> *spell_triggered = spellmgr.GetSpellLinked(id + SPELL_LINK_AURA))
            for(std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
                if(*itr < 0)
                    m_target->ApplySpellImmune(id, IMMUNITY_ID, -(*itr), m_target);
                else if(Unit* caster = GetCaster())
                    m_target->AddAura(*itr, m_target);
    }

    //*****************************************************
    // Update target aura state flag
    //*****************************************************

    // Update Seals information
    if (IsSealSpell(m_spellProto))
        SetAuraState(AURA_STATE_JUDGEMENT);

    // Conflagrate aura state on Immolate
    if (m_spellProto->SpellFamilyName == SPELLFAMILY_WARLOCK && m_spellProto->SpellFamilyFlags[0] & 4)
        SetAuraState(AURA_STATE_IMMOLATE);

    // Faerie Fire (druid versions)
    if (m_spellProto->SpellFamilyName == SPELLFAMILY_DRUID && m_spellProto->SpellFamilyFlags[0] & 0x400)
        SetAuraState(AURA_STATE_FAERIE_FIRE);

    // Victorious
    if (m_spellProto->SpellFamilyName == SPELLFAMILY_WARRIOR &&  m_spellProto->SpellFamilyFlags[1] & 0x00040000)
        SetAuraState(AURA_STATE_WARRIOR_VICTORY_RUSH);

    // Swiftmend state on Regrowth & Rejuvenation
    if (m_spellProto->SpellFamilyName == SPELLFAMILY_DRUID && m_spellProto->SpellFamilyFlags[0] & 0x50 )
        SetAuraState(AURA_STATE_SWIFTMEND);

    // Deadly poison aura state
    if(m_spellProto->SpellFamilyName == SPELLFAMILY_ROGUE && m_spellProto->SpellFamilyFlags[0] & 0x10000)
        SetAuraState(AURA_STATE_DEADLY_POISON);

    // Enrage aura state
    if(m_spellProto->Dispel == DISPEL_ENRAGE)
        SetAuraState(AURA_STATE_ENRAGE);

    if(GetSpellSchoolMask(m_spellProto) & SPELL_SCHOOL_MASK_FROST)
    {
        for (uint8 i = 0;i<MAX_SPELL_EFFECTS;++i)
        {
            if (m_spellProto->EffectApplyAuraName[i]==SPELL_AURA_MOD_STUN
                || m_spellProto->EffectApplyAuraName[i]==SPELL_AURA_MOD_ROOT)
            {
                SetAuraState(AURA_STATE_FROZEN);
                break;
            }
        }
    }

    m_target->ApplyModFlag(UNIT_FIELD_AURASTATE, GetAuraStateMask(), true);
}

bool Aura::SetPartAura(AuraEffect* aurEff, uint8 effIndex)
{
    if (IsRemoved())
        return false;
    if (m_auraFlags & (1<<effIndex))
        return false;
    m_auraFlags |= 1<<effIndex;
    m_partAuras[effIndex]=aurEff;
    m_target->HandleAuraEffect(aurEff, true);
    SendAuraUpdate();
    return true;
}

void Aura::_RemoveAura()
{
    Unit* caster = GetCaster();

    //passive auras do not get put in slots
    // Note: but totem can be not accessible for aura target in time remove (to far for find in grid)
    //if(m_isPassive && !(caster && caster->GetTypeId() == TYPEID_UNIT && ((Creature*)caster)->isTotem()))
    //    return;

    uint8 slot = GetAuraSlot();

    if (Aura * foundAura = m_target->GetAura(GetId(), GetCasterGUID()))
    {
        // allow use single slot only by auras from same caster
        slot = foundAura->GetAuraSlot();
        if(slot < MAX_AURAS)                                   // slot not set
            if (Aura *entry = m_target->GetVisibleAura(slot))
            {
                // set not valid slot for aura - prevent removing other visible aura
                slot = MAX_AURAS;
            }
    }

    // update for out of range group members
    if (slot < MAX_AURAS)
    {
        m_target->RemoveVisibleAura(slot);
        m_target->UpdateAuraForGroup(slot);
        SendAuraUpdate();
    }

    // unregister aura diminishing (and store last time)
    if (getDiminishGroup() != DIMINISHING_NONE )
        m_target->ApplyDiminishingAura(getDiminishGroup(),false);

    // Check needed only if aura applies aurastate
    if(GetAuraStateMask())
    {
        uint32 foundMask = 0;
        Unit::AuraMap& Auras = m_target->GetAuras();
        // Get mask of all aurastates from remaining auras
        for(Unit::AuraMap::iterator i = Auras.begin(); i != Auras.end(); ++i)
        {
            foundMask|=(*i).second->GetAuraStateMask();
        }
        // Remove only aurastates which were not found
        foundMask = GetAuraStateMask() &~foundMask;
        if (foundMask)
            m_target->ApplyModFlag(UNIT_FIELD_AURASTATE, foundMask, false);
    }

    // since now aura cannot apply/remove it's modifiers
    m_isRemoved = true;
    // disable client server communication for removed aura
    SetAuraSlot(MAX_AURAS);

    // reset cooldown state for spells
    if(caster && caster->GetTypeId() == TYPEID_PLAYER)
    {
        if ( GetSpellProto()->Attributes & SPELL_ATTR_DISABLED_WHILE_ACTIVE )
            // note: item based cooldowns and cooldown spell mods with charges ignored (unknown existed cases)
            ((Player*)caster)->SendCooldownEvent(GetSpellProto());
    }
    if (m_removeMode==AURA_REMOVE_BY_EXPIRE)
    {
        // Remove Linked Auras (on last aura remove)
        uint32 id = GetId();
        if(spellmgr.GetSpellCustomAttr(id) & SPELL_ATTR_CU_LINK_REMOVE)
        {
            if(const std::vector<int32> *spell_triggered = spellmgr.GetSpellLinked(-(int32)id))
                for(std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
                    if(*itr < 0)
                        m_target->RemoveAurasDueToSpell(-(*itr));
                    else if(Unit* caster = GetCaster())
                        m_target->CastSpell(m_target, *itr, true, 0, 0, caster->GetGUID());
        }
        if(spellmgr.GetSpellCustomAttr(id) & SPELL_ATTR_CU_LINK_AURA)
        {
            if(const std::vector<int32> *spell_triggered = spellmgr.GetSpellLinked(id + SPELL_LINK_AURA))
                for(std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
                    if(*itr < 0)
                        m_target->ApplySpellImmune(id, IMMUNITY_ID, -(*itr), false);
                    else
                        m_target->RemoveAurasDueToSpell(*itr);
        }
    }
    // Proc on aura remove (only spell flags for now)
    if (caster)
    {
        uint32 ProcCaster, ProcVictim;
        if (IsPositiveSpell(GetId()))
        {
            ProcCaster = PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL;
            ProcVictim = PROC_FLAG_TAKEN_POSITIVE_SPELL;
        }
        else
        {
            ProcCaster = PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT;
            ProcVictim = PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT;
        }
        uint32 procEx=0;
        if (m_removeMode == AURA_REMOVE_BY_ENEMY_SPELL)
            procEx = PROC_EX_AURA_REMOVE_DESTROY;
        else if (m_removeMode == AURA_REMOVE_BY_EXPIRE || m_removeMode == AURA_REMOVE_BY_CANCEL)
            procEx = PROC_EX_AURA_REMOVE_EXPIRE;
        else return;

        caster->ProcDamageAndSpell(m_target,ProcCaster, ProcVictim, procEx, m_procDamage, BASE_ATTACK, m_spellProto);
    }
}

void Aura::SetStackAmount(uint8 stackAmount)
{
    if (stackAmount != m_stackAmount)
    {
        Unit *target = GetTarget();
        Unit *caster = GetCaster();
        if (target && caster)
        {
            m_stackAmount = stackAmount;
            for (uint8 i=0;i<MAX_SPELL_EFFECTS;++i)
            {
                if (AuraEffect * part = GetPartAura(i))
                {
                    int32 amount = m_stackAmount * caster->CalculateSpellDamage(m_spellProto, part->GetEffIndex(), part->GetBasePoints(), target);
                    // Reapply if amount change
                    if (amount!=part->GetAmount())
                    {
                        bool Real = bool (part->m_spellmod);
                        // Auras which are applying spellmod should have removed spellmods for real
                        part->ApplyModifier(false,Real);
                        part->SetAmount(amount);
                        part->ApplyModifier(true, Real);
                    }
                }
            }
        }
    }
    RefreshAura();
}

bool Aura::modStackAmount(int32 num)
{
    // Can`t mod
    if (!m_spellProto->StackAmount)
        return true;

    // Modify stack but limit it
    int32 stackAmount = m_stackAmount + num;
    if (stackAmount > m_spellProto->StackAmount)
        stackAmount = m_spellProto->StackAmount;
    else if (stackAmount <=0) // Last aura from stack removed
    {
        m_stackAmount = 0;
        return true; // need remove aura
    }

    // Update stack amount
    SetStackAmount(stackAmount);
    return false;
}

void Aura::SetAuraDuration(int32 duration)
{
    m_duration = duration;
    //if (duration<0)
        //m_permanent=true;
    //else
        //m_permanent=false;
    SendAuraUpdate();
}

void Aura::SetAuraCharges(uint8 charges)
{
    if (m_procCharges == charges)
        return;
    m_procCharges = charges;
    SendAuraUpdate();
}

bool Aura::DropAuraCharge()
{
    if (m_procCharges == 0)
        return false;
    m_procCharges--;
    SendAuraUpdate();
    // return true if last charge dropped
    return m_procCharges == 0;
}

bool Aura::IsPersistent() const
{
    for (uint8 i=0; i<MAX_SPELL_EFFECTS;++i)
    {
        if (m_partAuras[i])
        {
            if (m_partAuras[i]->IsPersistent())
                return true;
        }
    }
    return false;
}

bool Aura::IsAreaAura() const
{
    for (uint8 i=0; i<MAX_SPELL_EFFECTS;++i)
    {
        if (m_partAuras[i])
        {
            if (m_partAuras[i]->IsAreaAura())
                return true;
        }
    }
    return false;
}

bool Aura::IsAuraType(AuraType type) const
{
    for (uint8 i=0; i<MAX_SPELL_EFFECTS;++i)
    {
        if (m_partAuras[i])
        {
            if (m_partAuras[i]->GetAuraName()==type)
                return true;
        }
    }
    return false;
}

void Aura::SetLoadedState(uint64 caster_guid,int32 maxduration,int32 duration,int32 charges, uint8 stackamount, int32 * amount)
{
    m_caster_guid = caster_guid;
    m_maxduration = maxduration;
    m_duration = duration;
    m_procCharges = charges;
    m_stackAmount = stackamount;
    for (uint8 i=0; i<MAX_SPELL_EFFECTS;++i)
        if (m_partAuras[i])
            m_partAuras[i]->SetAmount(amount[i]);
}

void AuraEffect::HandleShapeshiftBoosts(bool apply)
{
    uint32 spellId = 0;
    uint32 spellId2 = 0;
    uint32 HotWSpellId = 0;

    switch(GetMiscValue())
    {
        case FORM_CAT:
            spellId = 3025;
            HotWSpellId = 24900;
            break;
        case FORM_TREE:
            spellId = 5420;
            spellId2 = 34123;
            break;
        case FORM_TRAVEL:
            spellId = 5419;
            break;
        case FORM_AQUA:
            spellId = 5421;
            break;
        case FORM_BEAR:
            spellId = 1178;
            spellId2 = 21178;
            HotWSpellId = 24899;
            break;
        case FORM_DIREBEAR:
            spellId = 9635;
            spellId2 = 21178;
            HotWSpellId = 24899;
            break;
        case FORM_BATTLESTANCE:
            spellId = 21156;
            break;
        case FORM_DEFENSIVESTANCE:
            spellId = 7376;
            break;
        case FORM_BERSERKERSTANCE:
            spellId = 7381;
            break;
        case FORM_MOONKIN:
            spellId = 24905;
            // aura from effect trigger spell
            spellId2 = 24907;
            break;
        case FORM_FLIGHT:
            spellId = 33948;
            spellId2 = 34764;
            break;
        case FORM_FLIGHT_EPIC:
            spellId  = 40122;
            spellId2 = 40121;
            break;
        case FORM_METAMORPHOSIS:
            spellId  = 54817;
            spellId2 = 54879;
            break;
        case FORM_SPIRITOFREDEMPTION:
            spellId  = 27792;
            spellId2 = 27795;                               // must be second, this important at aura remove to prevent to early iterator invalidation.
            break;
        case FORM_GHOSTWOLF:
        case FORM_AMBIENT:
        case FORM_GHOUL:
        case FORM_SHADOW:
        case FORM_STEALTH:
        case FORM_CREATURECAT:
        case FORM_CREATUREBEAR:
            spellId = 0;
            break;
    }

    uint32 form = GetMiscValue()-1;

    if(apply)
    {
        if (spellId) m_target->CastSpell(m_target, spellId, true, NULL, this );
        if (spellId2) m_target->CastSpell(m_target, spellId2, true, NULL, this);

        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            const PlayerSpellMap& sp_list = ((Player *)m_target)->GetSpellMap();
            for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
            {
                if(itr->second->state == PLAYERSPELL_REMOVED) continue;
                if(itr->first==spellId || itr->first==spellId2) continue;
                SpellEntry const *spellInfo = sSpellStore.LookupEntry(itr->first);
                if (!spellInfo || !(spellInfo->Attributes & (SPELL_ATTR_PASSIVE | (1<<7)))) continue;
                if (spellInfo->Stances & (1<<form))
                    m_target->CastSpell(m_target, itr->first, true, NULL, this);
            }
            //LotP
            if (((Player*)m_target)->HasSpell(17007))
            {
                SpellEntry const *spellInfo = sSpellStore.LookupEntry(24932);
                if (spellInfo && spellInfo->Stances & (1<<form))
                    m_target->CastSpell(m_target, 24932, true, NULL, this);
            }
            // HotW
            if (HotWSpellId)
            {
                Unit::AuraEffectList const& mModTotalStatPct = m_target->GetAurasByType(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE);
                for(Unit::AuraEffectList::const_iterator i = mModTotalStatPct.begin(); i != mModTotalStatPct.end(); ++i)
                {
                    if ((*i)->GetSpellProto()->SpellIconID == 240 && (*i)->GetMiscValue() == 3)
                    {
                        int32 HotWMod = (*i)->GetAmount();
                        if(GetMiscValue() == FORM_CAT)
                            HotWMod /= 2;

                        m_target->CastCustomSpell(m_target, HotWSpellId, &HotWMod, NULL, NULL, true, NULL, this);
                        break;
                    }
                }
            }
        }
    }
    else
    {
        m_target->RemoveAurasDueToSpell(spellId);
        m_target->RemoveAurasDueToSpell(spellId2);

        Unit::AuraMap& tAuras = m_target->GetAuras();
        for (Unit::AuraMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
        {
            if (itr->second->IsRemovedOnShapeLost())
            {
                m_target->RemoveAura(itr);
            }
            else
            {
                ++itr;
            }
        }
    }
}

bool AuraEffect::isAffectedOnSpell(SpellEntry const *spell) const
{
    if (!spell)
        return false;
    // Check family name
    if (spell->SpellFamilyName != m_spellProto->SpellFamilyName)
        return false;
    // Check EffectClassMask
    if (m_spellProto->EffectSpellClassMask[m_effIndex] & spell->SpellFamilyFlags)
        return true;
    return false;
}

/*********************************************************/
/***               BASIC AURA FUNCTION                 ***/
/*********************************************************/
void AuraEffect::HandleAddModifier(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER || !Real)
        return;

    if(GetMiscValue() >= MAX_SPELLMOD)
        return;

    if (apply)
    {

        SpellModifier *mod = new SpellModifier;
        mod->op = SpellModOp(GetMiscValue());
        mod->value = m_amount;
        mod->type = SpellModType(m_auraName);    // SpellModType value == spell aura types
        mod->spellId = GetId();

        flag96 const *spellAffect = spellmgr.GetSpellAffect(GetId(), m_effIndex);
        if (!spellAffect)
            spellAffect = &m_spellProto->EffectSpellClassMask[m_effIndex];
        mod->mask = *spellAffect;
        mod->charges = GetParentAura()->GetAuraCharges();

        m_spellmod = mod;
    }

    ((Player*)m_target)->AddSpellMod(m_spellmod, apply);

    // reapply some passive spells after add/remove related spellmods
    if(m_spellProto->SpellFamilyName==SPELLFAMILY_WARRIOR && (m_spellmod->mask[1] & 0x00001000))
    {
        m_target->RemoveAurasDueToSpell(45471);

        if(apply)
            m_target->CastSpell(m_target,45471,true);
    }
}
void AuraEffect::HandleAddTargetTrigger(bool apply, bool Real)
{
    // Use SpellModifier structure for check
    // used only fields:
    //  spellId, mask, mask2
    if (apply)
    {
        SpellModifier *mod = new SpellModifier;
        mod->spellId = GetId();

        flag96 const *spellAffect = spellmgr.GetSpellAffect(GetId(), m_effIndex);
        if (!spellAffect)
            spellAffect = &m_spellProto->EffectSpellClassMask[m_effIndex];

        mod->mask = *spellAffect;
        m_spellmod = mod;
    }
    else
    {
        delete m_spellmod;
        m_spellmod = NULL;
    }
}

void AuraEffect::TriggerSpell()
{
    Unit* caster = GetCaster();
    Unit* target = GetTriggerTarget();

    if(!caster || !target)
        return;

    // generic casting code with custom spells and target/caster customs
    uint32 trigger_spell_id = GetSpellProto()->EffectTriggerSpell[m_effIndex];

    SpellEntry const *triggeredSpellInfo = sSpellStore.LookupEntry(trigger_spell_id);
    SpellEntry const *auraSpellInfo = GetSpellProto();
    uint32 auraId = auraSpellInfo->Id;

    // specific code for cases with no trigger spell provided in field
    if (triggeredSpellInfo == NULL)
    {
        switch(auraSpellInfo->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
            {
                switch(auraId)
                {
                    // Firestone Passive (1-5 ranks)
                    case 758:
                    case 17945:
                    case 17947:
                    case 17949:
                    case 27252:
                    {
                        if (caster->GetTypeId()!=TYPEID_PLAYER)
                            return;
                        Item* item = ((Player*)caster)->GetWeaponForAttack(BASE_ATTACK);
                        if (!item)
                            return;
                        uint32 enchant_id = 0;
                        switch (GetId())
                        {
                             case   758: enchant_id = 1803; break;   // Rank 1
                             case 17945: enchant_id = 1823; break;   // Rank 2
                             case 17947: enchant_id = 1824; break;   // Rank 3
                             case 17949: enchant_id = 1825; break;   // Rank 4
                             case 27252: enchant_id = 2645; break;   // Rank 5
                             default:
                                 return;
                        }
                        // remove old enchanting before applying new
                        ((Player*)caster)->ApplyEnchantment(item,TEMP_ENCHANTMENT_SLOT,false);
                        item->SetEnchantment(TEMP_ENCHANTMENT_SLOT, enchant_id, m_amplitude+1000, 0);
                        // add new enchanting
                        ((Player*)caster)->ApplyEnchantment(item,TEMP_ENCHANTMENT_SLOT,true);
                        return;
                    }
//                    // Periodic Mana Burn
//                    case 812: break;
//                    // Polymorphic Ray
//                    case 6965: break;
//                    // Fire Nova (1-7 ranks)
//                    case 8350:
//                    case 8508:
//                    case 8509:
//                    case 11312:
//                    case 11313:
//                    case 25540:
//                    case 25544:
//                        break;
                    // Thaumaturgy Channel
                    case 9712: trigger_spell_id = 21029; break;
//                    // Egan's Blaster
//                    case 17368: break;
//                    // Haunted
//                    case 18347: break;
//                    // Ranshalla Waiting
//                    case 18953: break;
//                    // Inferno
//                    case 19695: break;
//                    // Frostwolf Muzzle DND
//                    case 21794: break;
//                    // Alterac Ram Collar DND
//                    case 21866: break;
//                    // Celebras Waiting
//                    case 21916: break;
                    // Brood Affliction: Bronze
                    case 23170:
                    {
                        m_target->CastSpell(m_target, 23171, true, 0, this);
                        return;
                    }
//                    // Mark of Frost
//                    case 23184: break;
                    // Restoration
                    case 23493:
                    {
                        int32 heal = caster->GetMaxHealth() / 10;
                        caster->ModifyHealth( heal );
                        caster->SendHealSpellLog(caster, 23493, heal);

                        int32 mana = caster->GetMaxPower(POWER_MANA);
                        if (mana)
                        {
                            mana /= 10;
                            caster->ModifyPower( POWER_MANA, mana );
                            caster->SendEnergizeSpellLog(caster, 23493, mana, POWER_MANA);
                        }
                        return;
                    }
//                    // Stoneclaw Totem Passive TEST
//                    case 23792: break;
//                    // Axe Flurry
//                    case 24018: break;
//                    // Mark of Arlokk
//                    case 24210: break;
//                    // Restoration
//                    case 24379: break;
//                    // Happy Pet
//                    case 24716: break;
//                    // Dream Fog
//                    case 24780: break;
//                    // Cannon Prep
//                    case 24832: break;
//                    // Shadow Bolt Whirl
//                    case 24834: break;
//                    // Stink Trap
//                    case 24918: break;
//                    // Mark of Nature
//                    case 25041: break;
//                    // Agro Drones
//                    case 25152: break;
//                    // Consume
//                    case 25371: break;
//                    // Pain Spike
//                    case 25572: break;
//                    // Rotate 360
//                    case 26009: break;
//                    // Rotate -360
//                    case 26136: break;
//                    // Consume
//                    case 26196: break;
//                    // Berserk
//                    case 26615: break;
//                    // Defile
//                    case 27177: break;
//                    // Teleport: IF/UC
//                    case 27601: break;
//                    // Five Fat Finger Exploding Heart Technique
//                    case 27673: break;
                    // Nitrous Boost
                    case 27746:
                    {
                        if (caster->GetPower(POWER_MANA) >= 10)
                        {
                            caster->ModifyPower( POWER_MANA, -10 );
                            caster->SendEnergizeSpellLog(caster, 27746, -10, POWER_MANA);
                        } else
                        {
                            caster->RemoveAurasDueToSpell(27746);
                            return;
                        }
                    } break;
//                    // Steam Tank Passive
//                    case 27747: break;
                    // Frost Blast
                    case 27808:
                    {
                        int32 bpDamage = target->GetMaxHealth()*26/100;
                        caster->CastCustomSpell(target,29879,&bpDamage,NULL,NULL,true,NULL,this);
                        return;
                    }
//                    // Detonate Mana
//                    case 27819: break;
//                    // Controller Timer
//                    case 28095: break;
//                    // Stalagg Chain
//                    case 28096: break;
//                    // Stalagg Tesla Passive
//                    case 28097: break;
//                    // Feugen Tesla Passive
//                    case 28109: break;
//                    // Feugen Chain
//                    case 28111: break;
//                    // Mark of Didier
//                    case 28114: break;
//                    // Communique Timer, camp
//                    case 28346: break;
//                    // Icebolt
//                    case 28522: break;
//                    // Silithyst
//                    case 29519: break;
                    // Inoculate Nestlewood Owlkin
                    case 29528:
                        if(target->GetTypeId()!=TYPEID_UNIT)// prevent error reports in case ignored player target
                            return;
                        break;
//                    // Overload
//                    case 29768: break;
//                    // Return Fire
//                    case 29788: break;
//                    // Return Fire
//                    case 29793: break;
//                    // Return Fire
//                    case 29794: break;
//                    // Guardian of Icecrown Passive
//                    case 29897: break;
                    // Feed Captured Animal
                    case 29917: trigger_spell_id = 29916; break;
//                    // Flame Wreath
//                    case 29946: break;
//                    // Flame Wreath
//                    case 29947: break;
//                    // Mind Exhaustion Passive
//                    case 30025: break;
//                    // Nether Beam - Serenity
//                    case 30401: break;
                    // Extract Gas
                    case 30427:
                    {
                        // move loot to player inventory and despawn target
                        if(caster->GetTypeId() ==TYPEID_PLAYER &&
                                target->GetTypeId() == TYPEID_UNIT &&
                                ((Creature*)target)->GetCreatureInfo()->type == CREATURE_TYPE_GAS_CLOUD)
                        {
                            Player* player = (Player*)caster;
                            Creature* creature = (Creature*)target;
                            // missing lootid has been reported on startup - just return
                            if (!creature->GetCreatureInfo()->SkinLootId)
                                return;

                            player->AutoStoreLoot(creature->GetCreatureInfo()->SkinLootId,LootTemplates_Skinning,true);

                            creature->setDeathState(JUST_DIED);
                            creature->RemoveCorpse();
                            creature->SetHealth(0);         // just for nice GM-mode view
                        }
                        return;
                    }
                    // Quake
                    case 30576: trigger_spell_id = 30571; break;
//                    // Burning Maul
//                    case 30598: break;
//                    // Regeneration
//                    case 30799:
//                    case 30800:
//                    case 30801:
//                        break;
//                    // Despawn Self - Smoke cloud
//                    case 31269: break;
//                    // Time Rift Periodic
//                    case 31320: break;
//                    // Corrupt Medivh
//                    case 31326: break;
                    // Doom
                    case 31347:
                    {
                        m_target->CastSpell(m_target,31350,true);
                        m_target->DealDamage(m_target, m_target->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                        return;
                    }
                    // Spellcloth
                    case 31373:
                    {
                        // Summon Elemental after create item
                        caster->SummonCreature(17870, 0, 0, 0, caster->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
                        return;
                    }
//                    // Bloodmyst Tesla
//                    case 31611: break;
//                    // Doomfire
//                    case 31944: break;
//                    // Teleport Test
//                    case 32236: break;
//                    // Earthquake
//                    case 32686: break;
//                    // Possess
//                    case 33401: break;
//                    // Draw Shadows
//                    case 33563: break;
//                    // Murmur's Touch
//                    case 33711: break;
                    // Flame Quills
                    case 34229:
                    {
                        // cast 24 spells 34269-34289, 34314-34316
                        for(uint32 spell_id = 34269; spell_id != 34290; ++spell_id)
                            caster->CastSpell(m_target,spell_id,true);
                        for(uint32 spell_id = 34314; spell_id != 34317; ++spell_id)
                            caster->CastSpell(m_target,spell_id,true);
                        return;
                    }
//                    // Gravity Lapse
//                    case 34480: break;
//                    // Tornado
//                    case 34683: break;
//                    // Frostbite Rotate
//                    case 34748: break;
//                    // Arcane Flurry
//                    case 34821: break;
//                    // Interrupt Shutdown
//                    case 35016: break;
//                    // Interrupt Shutdown
//                    case 35176: break;
//                    // Inferno
//                    case 35268: break;
//                    // Salaadin's Tesla
//                    case 35515: break;
//                    // Ethereal Channel (Red)
//                    case 35518: break;
//                    // Nether Vapor
//                    case 35879: break;
//                    // Dark Portal Storm
//                    case 36018: break;
//                    // Burning Maul
//                    case 36056: break;
//                    // Living Grove Defender Lifespan
//                    case 36061: break;
//                    // Professor Dabiri Talks
//                    case 36064: break;
//                    // Kael Gaining Power
//                    case 36091: break;
//                    // They Must Burn Bomb Aura
//                    case 36344: break;
//                    // They Must Burn Bomb Aura (self)
//                    case 36350: break;
//                    // Stolen Ravenous Ravager Egg
//                    case 36401: break;
//                    // Activated Cannon
//                    case 36410: break;
//                    // Stolen Ravenous Ravager Egg
//                    case 36418: break;
//                    // Enchanted Weapons
//                    case 36510: break;
//                    // Cursed Scarab Periodic
//                    case 36556: break;
//                    // Cursed Scarab Despawn Periodic
//                    case 36561: break;
//                    // Vision Guide
//                    case 36573: break;
//                    // Cannon Charging (platform)
//                    case 36785: break;
//                    // Cannon Charging (self)
//                    case 36860: break;
                    // Remote Toy
                    case 37027: trigger_spell_id = 37029; break;
//                    // Mark of Death
//                    case 37125: break;
//                    // Arcane Flurry
//                    case 37268: break;
//                    // Spout
//                    case 37429: break;
//                    // Spout
//                    case 37430: break;
//                    // Karazhan - Chess NPC AI, Snapshot timer
//                    case 37440: break;
//                    // Karazhan - Chess NPC AI, action timer
//                    case 37504: break;
//                    // Karazhan - Chess: Is Square OCCUPIED aura (DND)
//                    case 39400: break;
//                    // Banish
//                    case 37546: break;
//                    // Shriveling Gaze
//                    case 37589: break;
//                    // Fake Aggro Radius (2 yd)
//                    case 37815: break;
//                    // Corrupt Medivh
//                    case 37853: break;
                    // Eye of Grillok
                    case 38495:
                    {
                        m_target->CastSpell(m_target, 38530, true);
                        return;
                    }
                    // Absorb Eye of Grillok (Zezzak's Shard)
                    case 38554:
                    {
                        if(m_target->GetTypeId() != TYPEID_UNIT)
                            return;

                        caster->CastSpell(caster, 38495, true);

                        Creature* creatureTarget = (Creature*)m_target;

                        creatureTarget->setDeathState(JUST_DIED);
                        creatureTarget->RemoveCorpse();
                        creatureTarget->SetHealth(0);       // just for nice GM-mode view
                        return;
                    }
//                    // Magic Sucker Device timer
//                    case 38672: break;
//                    // Tomb Guarding Charging
//                    case 38751: break;
//                    // Murmur's Touch
//                    case 38794: break;
//                    // Activate Nether-wraith Beacon (31742 Nether-wraith Beacon item)
//                    case 39105: break;
//                    // Drain World Tree Visual
//                    case 39140: break;
//                    // Quest - Dustin's Undead Dragon Visual aura
//                    case 39259: break;
//                    // Hellfire - The Exorcism, Jules releases darkness, aura
//                    case 39306: break;
//                    // Inferno
//                    case 39346: break;
//                    // Enchanted Weapons
//                    case 39489: break;
//                    // Shadow Bolt Whirl
//                    case 39630: break;
//                    // Shadow Bolt Whirl
//                    case 39634: break;
//                    // Shadow Inferno
//                    case 39645: break;
                    // Tear of Azzinoth Summon Channel - it's not really supposed to do anything,and this only prevents the console spam
                    case 39857: trigger_spell_id = 39856; break;
//                    // Soulgrinder Ritual Visual (Smashed)
//                    case 39974: break;
//                    // Simon Game Pre-game timer
//                    case 40041: break;
//                    // Knockdown Fel Cannon: The Aggro Check Aura
//                    case 40113: break;
//                    // Spirit Lance
//                    case 40157: break;
//                    // Demon Transform 2
//                    case 40398: break;
//                    // Demon Transform 1
//                    case 40511: break;
//                    // Ancient Flames
//                    case 40657: break;
//                    // Ethereal Ring Cannon: Cannon Aura
//                    case 40734: break;
//                    // Cage Trap
//                    case 40760: break;
//                    // Random Periodic
//                    case 40867: break;
//                    // Prismatic Shield
//                    case 40879: break;
                    // Aura of Desire
                    case 41350:
                    {
                        Unit::AuraEffectList const& mMod = m_target->GetAurasByType(SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT);
                        for(Unit::AuraEffectList::const_iterator i = mMod.begin(); i != mMod.end(); ++i)
                        {
                            if ((*i)->GetId() == 41350)
                            {
                                (*i)->ApplyModifier(false);
                                (*i)->SetAmount((*i)->GetAmount()-5);
                                (*i)->ApplyModifier(true);
                                break;
                            }
                        }
                    }break;
//                    // Dementia
//                    case 41404: break;
//                    // Chaos Form
//                    case 41629: break;
//                    // Alert Drums
//                    case 42177: break;
//                    // Spout
//                    case 42581: break;
//                    // Spout
//                    case 42582: break;
//                    // Return to the Spirit Realm
//                    case 44035: break;
//                    // Curse of Boundless Agony
//                    case 45050: break;
//                    // Earthquake
//                    case 46240: break;
                    // Personalized Weather
                    case 46736: trigger_spell_id = 46737; break;
//                    // Stay Submerged
//                    case 46981: break;
//                    // Dragonblight Ram
//                    case 47015: break;
//                    // Party G.R.E.N.A.D.E.
//                    case 51510: break;
                    default:
                        break;
                }
                break;
            }
            case SPELLFAMILY_MAGE:
            {
                switch(auraId)
                {
                    // Invisibility
                    case 66:
                    // Here need periodic triger reducing threat spell (or do it manually)
                        return;
                    default:
                        break;
                }
                break;
            }
//            case SPELLFAMILY_WARRIOR:
//            {
//                switch(auraId)
//                {
//                    // Wild Magic
//                    case 23410: break;
//                    // Corrupted Totems
//                    case 23425: break;
//                    default:
//                        break;
//                }
//                break;
//            }
//            case SPELLFAMILY_PRIEST:
//            {
//                switch(auraId)
//                {
//                    // Blue Beam
//                    case 32930: break;
//                    // Fury of the Dreghood Elders
//                    case 35460: break;
//                    default:
//                        break;
//                }
 //               break;
 //           }
            case SPELLFAMILY_DRUID:
            {
                switch(auraId)
                {
                    // Cat Form
                    // trigger_spell_id not set and unknown effect triggered in this case, ignoring for while
                    case 768:
                        return;
                    // Frenzied Regeneration
                    case 22842:
                    case 22895:
                    case 22896:
                    case 26999:
                    {
                        int32 LifePerRage = GetAmount();

                        int32 lRage = m_target->GetPower(POWER_RAGE);
                        if(lRage > 100)                                     // rage stored as rage*10
                            lRage = 100;
                        m_target->ModifyPower(POWER_RAGE, -lRage);
                        int32 FRTriggerBasePoints = int32(lRage*LifePerRage/10);
                        m_target->CastCustomSpell(m_target,22845,&FRTriggerBasePoints,NULL,NULL,true,NULL,this);
                        return;
                    }
                    default:
                        break;
                }
                break;
            }

//            case SPELLFAMILY_HUNTER:
//            {
//                switch(auraId)
//                {
//                    //Frost Trap Aura
//                    case 13810:
//                        return;
//                    //Rizzle's Frost Trap
//                    case 39900:
//                        return;
//                    // Tame spells
//                    case 19597:         // Tame Ice Claw Bear
//                    case 19676:         // Tame Snow Leopard
//                    case 19677:         // Tame Large Crag Boar
//                    case 19678:         // Tame Adult Plainstrider
//                    case 19679:         // Tame Prairie Stalker
//                    case 19680:         // Tame Swoop
//                    case 19681:         // Tame Dire Mottled Boar
//                    case 19682:         // Tame Surf Crawler
//                    case 19683:         // Tame Armored Scorpid
//                    case 19684:         // Tame Webwood Lurker
//                    case 19685:         // Tame Nightsaber Stalker
//                    case 19686:         // Tame Strigid Screecher
//                    case 30100:         // Tame Crazed Dragonhawk
//                    case 30103:         // Tame Elder Springpaw
//                    case 30104:         // Tame Mistbat
//                    case 30647:         // Tame Barbed Crawler
//                    case 30648:         // Tame Greater Timberstrider
//                    case 30652:         // Tame Nightstalker
//                        return;
//                    default:
//                        break;
//                }
//                break;
//            }
            case SPELLFAMILY_SHAMAN:
            {
                switch(auraId)
                {
                    // Lightning Shield (The Earthshatterer set trigger after cast Lighting Shield)
                    case 28820:
                    {
                        // Need remove self if Lightning Shield not active
                        Unit::AuraMap const& auras = target->GetAuras();
                        for(Unit::AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
                        {
                            SpellEntry const* spell = itr->second->GetSpellProto();
                            if( spell->SpellFamilyName == SPELLFAMILY_SHAMAN &&
                                spell->SpellFamilyFlags[0] & 0x400)
                                return;
                        }
                        target->RemoveAurasDueToSpell(28820);
                        return;
                    }
                    // Totemic Mastery (Skyshatter Regalia (Shaman Tier 6) - bonus)
                    case 38443:
                    {
                        bool all = true;
                        for(int i = SUMMON_SLOT_TOTEM; i < MAX_TOTEM_SLOT; ++i)
                        {
                            if(!caster->m_SummonSlot[i])
                            {
                                all = false;
                                break;
                            }
                        }

                        if(all)
                            caster->CastSpell(caster,38437,true);
                        else
                            caster->RemoveAurasDueToSpell(38437);
                        return;
                    }
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }

        // Reget trigger spell proto
        triggeredSpellInfo = sSpellStore.LookupEntry(trigger_spell_id);
    }
    else
    {
        // Spell exist but require custom code
        switch(auraId)
        {
            // Curse of Idiocy
            case 1010:
            {
                // TODO: spell casted by result in correct way mostly
                // BUT:
                // 1) target show casting at each triggered cast: target don't must show casting animation for any triggered spell
                //      but must show affect apply like item casting
                // 2) maybe aura must be replace by new with accumulative stat mods instead stacking

                // prevent cast by triggered auras
                if(GetCasterGUID() == m_target->GetGUID())
                    return;

                // stop triggering after each affected stats lost > 90
                int32 intellectLoss = 0;
                int32 spiritLoss = 0;

                Unit::AuraEffectList const& mModStat = m_target->GetAurasByType(SPELL_AURA_MOD_STAT);
                for(Unit::AuraEffectList::const_iterator i = mModStat.begin(); i != mModStat.end(); ++i)
                {
                    if ((*i)->GetId() == 1010)
                    {
                        switch((*i)->GetMiscValue())
                        {
                            case STAT_INTELLECT: intellectLoss += (*i)->GetAmount(); break;
                            case STAT_SPIRIT:    spiritLoss   += (*i)->GetAmount(); break;
                            default: break;
                        }
                    }
                }

                if(intellectLoss <= -90 && spiritLoss <= -90)
                    return;

                caster = target;
                break;
            }
            // Mana Tide
            case 16191:
            {
                caster->CastCustomSpell(target, trigger_spell_id, &m_amount, NULL, NULL, true, NULL, this);
                return;
            }
        }
    }

    if(triggeredSpellInfo)
    {
        if(!caster->GetSpellMaxRangeForTarget(m_target,sSpellRangeStore.LookupEntry(triggeredSpellInfo->rangeIndex)))
            target = m_target;    //for druid dispel poison
        m_target->CastSpell(target, triggeredSpellInfo, true, 0, this, GetCasterGUID());
    }
    else if(target->GetTypeId()!=TYPEID_UNIT || !Script->EffectDummyCreature(caster, GetId(), GetEffIndex(), (Creature*)target))
        sLog.outError("AuraEffect::TriggerSpell: Spell %u have 0 in EffectTriggered[%d], not handled custom case?",GetId(),GetEffIndex());
}

Unit* AuraEffect::GetTriggerTarget() const
{
    Unit* target = ObjectAccessor::GetUnit(*m_target, m_target->GetUInt64Value(UNIT_FIELD_TARGET));
    return target ? target : m_target;
}

void AuraEffect::TriggerSpellWithValue()
{
    Unit* caster = GetCaster();
    Unit* target = GetTriggerTarget();

    if(!caster || !target)
        return;

    // generic casting code with custom spells and target/caster customs
    uint32 trigger_spell_id = GetSpellProto()->EffectTriggerSpell[m_effIndex];
    int32  basepoints0 = this->GetAmount();

    caster->CastCustomSpell(target, trigger_spell_id, &basepoints0, 0, 0, true, 0, this);
}

/*********************************************************/
/***                  AURA EFFECTS                     ***/
/*********************************************************/

void AuraEffect::HandleAuraDummy(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    Unit* caster = GetCaster();

    // AT APPLY
    if(apply)
    {
        switch(GetId())
        {
            case 1515:                                      // Tame beast
                // FIX_ME: this is 2.0.12 threat effect replaced in 2.1.x by dummy aura, must be checked for correctness
                if( caster && m_target->CanHaveThreatList())
                    m_target->AddThreat(caster, 10.0f);
                return;
            case 13139:                                     // net-o-matic
                // root to self part of (root_target->charge->root_self sequence
                if(caster)
                    caster->CastSpell(caster,13138,true,NULL,this);
                return;
            case 34026:   // kill command
            {
                Unit * pet = m_target->GetGuardianPet();
                if (!pet)
                    return;

                m_target->CastSpell(m_target,34027,true,NULL,this);

                // set 3 stacks and 3 charges (to make all auras not disappear at once)
                Aura* owner_aura = m_target->GetAura(34027,GetCasterGUID());
                Aura* pet_aura  = pet->GetAura(58914, GetCasterGUID());
                if( owner_aura )
                {
                    owner_aura->SetStackAmount(owner_aura->GetSpellProto()->StackAmount);
                }
                if( pet_aura )
                {
                    pet_aura->SetAuraCharges(0);
                    pet_aura->SetStackAmount(owner_aura->GetSpellProto()->StackAmount);
                }
                return;
            }
            case 55198:   // Tidal Force
            {
                m_target->CastSpell(m_target,55166,true,NULL,this);
                // set 3 stacks and 3 charges (to make all auras not disappear at once)
                Aura* owner_aura = m_target->GetAura(55166,GetCasterGUID());
                if( owner_aura )
                {
                    // This aura lasts 2 sec, need this hack to properly proc spells
                    // TODO: drop aura charges for ApplySpellMod in ProcDamageAndSpell
                    GetParentAura()->SetAuraDuration(owner_aura->GetAuraDuration());
                    // Make aura be not charged-this prevents removing charge on not crit spells
                    owner_aura->SetAuraCharges(0);
                    owner_aura->SetStackAmount(owner_aura->GetSpellProto()->StackAmount);
                }
                return;
            }
            case 39850:                                     // Rocket Blast
                if(roll_chance_i(20))                       // backfire stun
                    m_target->CastSpell(m_target, 51581, true, NULL, this);
                return;
            case 43873:                                     // Headless Horseman Laugh
                m_target->PlayDistanceSound(11965);
                return;
            case 46354:                                     // Blood Elf Illusion
                if(caster)
                {
                    switch(caster->getGender())
                    {
                        case GENDER_FEMALE:
                            caster->CastSpell(m_target,46356,true,NULL,this);
                            break;
                        case GENDER_MALE:
                            caster->CastSpell(m_target,46355,true,NULL,this);
                            break;
                        default:
                            break;
                    }
                }
                return;
            case 46699:                                     // Requires No Ammo
                if(m_target->GetTypeId()==TYPEID_PLAYER)
                    ((Player*)m_target)->RemoveAmmo();      // not use ammo and not allow use
                return;
        }

        // Earth Shield
        if ( caster && GetSpellProto()->SpellFamilyName == SPELLFAMILY_SHAMAN && (GetSpellProto()->SpellFamilyFlags[1] & 0x400))
        {
            // prevent double apply bonuses
            if(m_target->GetTypeId()!=TYPEID_PLAYER || !((Player*)m_target)->GetSession()->PlayerLoading())
                m_amount = caster->SpellHealingBonus(m_target, GetSpellProto(), m_amount, SPELL_DIRECT_DAMAGE);
            return;
        }
        //Druid, Survival Instincts
        if(GetSpellProto()->SpellFamilyName==SPELLFAMILY_DRUID && GetSpellProto()->SpellFamilyFlags[2]& 0x40 )
        {
            if(!m_target)
                return;

               int32 bp0 = int32(m_target->GetMaxHealth() * m_amount / 100);
               m_target->CastCustomSpell(m_target, 50322, &bp0, NULL, NULL, true);
        }
    }
    // AT REMOVE
    else
    {
        if( (IsQuestTameSpell(GetId())) && caster && caster->isAlive() && m_target->isAlive())
        {
            uint32 finalSpelId = 0;
            switch(GetId())
            {
                case 19548: finalSpelId = 19597; break;
                case 19674: finalSpelId = 19677; break;
                case 19687: finalSpelId = 19676; break;
                case 19688: finalSpelId = 19678; break;
                case 19689: finalSpelId = 19679; break;
                case 19692: finalSpelId = 19680; break;
                case 19693: finalSpelId = 19684; break;
                case 19694: finalSpelId = 19681; break;
                case 19696: finalSpelId = 19682; break;
                case 19697: finalSpelId = 19683; break;
                case 19699: finalSpelId = 19685; break;
                case 19700: finalSpelId = 19686; break;
                case 30646: finalSpelId = 30647; break;
                case 30653: finalSpelId = 30648; break;
                case 30654: finalSpelId = 30652; break;
                case 30099: finalSpelId = 30100; break;
                case 30102: finalSpelId = 30103; break;
                case 30105: finalSpelId = 30104; break;
            }

            if(finalSpelId)
                caster->CastSpell(m_target,finalSpelId,true,NULL,this);
            return;
        }

        // Waiting to Resurrect
        if(GetId()==2584)
        {
            // Waiting to resurrect spell cancel, we must remove player from resurrect queue
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                if(BattleGround *bg = ((Player*)m_target)->GetBattleGround())
                    bg->RemovePlayerFromResurrectQueue(m_target->GetGUID());
            return;
        }

        // Burning Winds
        if(GetId()==46308)                                  // casted only at creatures at spawn
        {
            m_target->CastSpell(m_target,47287,true,NULL,this);
            return;
        }

        // Misdirection
        if(GetId()==34477)
        {
            m_target->SetReducedThreatPercent(0, 0);
            return;
        }

        switch(m_spellProto->SpellFamilyName)
        {
            case SPELLFAMILY_WARLOCK:
                // Haunt
                if(m_spellProto->SpellFamilyFlags[1] & 0x40000)
                {
                    if(caster)
                        caster->CastCustomSpell(caster, 48210, &m_currentBasePoints, 0, 0, true);
                    return;
                }
                break;
            case SPELLFAMILY_MAGE:
                // Living Bomb
                if(m_spellProto->SpellFamilyFlags[1] & 0x20000)
                {
                    if(caster && (GetParentAura()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL || GetParentAura()->GetRemoveMode() == AURA_REMOVE_BY_DEATH || GetParentAura()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE))
                        caster->CastSpell(m_target, GetAmount(), true);
                    return;
                }
                // Focused Magic
                if(m_spellProto->Id == 54646)
                {
                    // only on remove by crit
                    if(caster && GetParentAura()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                        caster->CastSpell(caster,54648, true);
                    return;
                }
                break;
        }
    }

    // AT APPLY & REMOVE

    switch(m_spellProto->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            // Unstable Power
            if( GetId()==24658 )
            {
                uint32 spellId = 24659;
                if (apply)
                {
                    const SpellEntry *spell = sSpellStore.LookupEntry(spellId);
                    if (!spell)
                        return;
                    for (int i=0; i < spell->StackAmount; ++i)
                        caster->CastSpell(m_target, spell->Id, true, NULL, NULL, GetCasterGUID());
                    return;
                }
                m_target->RemoveAurasDueToSpell(spellId);
                return;
            }
            // Restless Strength
            if( GetId()==24661 )
            {
                uint32 spellId = 24662;
                if (apply)
                {
                    const SpellEntry *spell = sSpellStore.LookupEntry(spellId);
                    if (!spell)
                        return;
                    for (int i=0; i < spell->StackAmount; ++i)
                        caster->CastSpell(m_target, spell->Id, true, NULL, NULL, GetCasterGUID());
                    return;
                }
                m_target->RemoveAurasDueToSpell(spellId);
                return;
            }
            //Summon Fire Elemental
            if (GetId() == 40133 && caster)
            {
                Unit *owner = caster->GetOwner();
                if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                {
                    if(apply)
                        owner->CastSpell(owner,8985,true);
                    else
                        ((Player*)owner)->RemovePet(NULL, PET_SAVE_NOT_IN_SLOT, true);
                }
                return;
            }

            //Summon Earth Elemental
            if (GetId() == 40132 && caster)
            {
                Unit *owner = caster->GetOwner();
                if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                {
                    if(apply)
                        owner->CastSpell(owner,19704,true);
                    else
                        ((Player*)owner)->RemovePet(NULL, PET_SAVE_NOT_IN_SLOT, true);
                }
                return;
            }

            break;
        }
        case SPELLFAMILY_MAGE:
        {
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            // Pain and Suffering
            if( m_spellProto->SpellIconID == 2874 && m_target->GetTypeId()==TYPEID_PLAYER )
            {
                if(apply)
                {
                    // Reduce backfire damage (dot damage) from Shadow Word: Death
                    SpellModifier *mod = new SpellModifier;
                    mod->op = SPELLMOD_DOT;
                    mod->value = m_amount;
                    mod->type = SPELLMOD_PCT;
                    mod->spellId = GetId();
                    mod->mask[1] = 0x00002000;
                    m_spellmod = mod;
                }
                ((Player*)m_target)->AddSpellMod(m_spellmod, apply);
                return;
            }
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            switch(GetId())
            {
                case 34246:                                 // Idol of the Emerald Queen
                {
                    if (m_target->GetTypeId() != TYPEID_PLAYER)
                        return;

                    if(apply)
                    {
                        SpellModifier *mod = new SpellModifier;
                        mod->op = SPELLMOD_DOT;
                        mod->value = m_amount/7;
                        mod->type = SPELLMOD_FLAT;
                        mod->spellId = GetId();
                        mod->mask[1] = 0x0010;

                        m_spellmod = mod;
                    }

                    ((Player*)m_target)->AddSpellMod(m_spellmod, apply);
                    return;
                }
                case 61336:                                 // Survival Instincts
                {
                    if(apply)
                    {
                        if (!m_target->IsInFeralForm())
                            return;

                        int32 bp0 = int32(m_target->GetMaxHealth() * m_amount / 100);
                        m_target->CastCustomSpell(m_target, 50322, &bp0, NULL, NULL, true);
                    }
                    else
                        m_target-> RemoveAurasDueToSpell(50322);
                    return;
                }
            }

            // Lifebloom
            if ( GetSpellProto()->SpellFamilyFlags[1] & 0x10 )
            {
                if ( apply )
                {
                    if ( caster )
                        // prevent double apply bonuses
                        if(m_target->GetTypeId()!=TYPEID_PLAYER || !((Player*)m_target)->GetSession()->PlayerLoading())
                            m_amount = caster->SpellHealingBonus(m_target, GetSpellProto(), m_amount, SPELL_DIRECT_DAMAGE);
                }
                else
                {
                    // Final heal only on dispelled or duration end
                    if (GetParentAura()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE && GetParentAura()->GetRemoveMode() != AURA_REMOVE_BY_ENEMY_SPELL)
                        return;

                    // final heal
                    if(m_target->IsInWorld())
                        m_target->CastCustomSpell(m_target,33778,&m_amount,NULL,NULL,true,NULL,this,GetCasterGUID());

                    /*// have a look if there is still some other Lifebloom dummy aura
                    Unit::AuraList auras = m_target->GetAurasByType(SPELL_AURA_DUMMY);
                    for(Unit::AuraList::iterator itr = auras.begin(); itr!=auras.end(); ++itr)
                        if((*itr)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_DRUID &&
                            (*itr)->GetSpellProto()->SpellFamilyFlags & 0x1000000000LL)
                            return;

                    // final heal
                    if(m_target->IsInWorld() && m_stackAmount > 0)
                    {
                        int32 amount = m_amount / m_stackAmount;
                        m_target->CastCustomSpell(m_target,33778,&amount,NULL,NULL,true,NULL,this,GetCasterGUID());
                    }*/
                }
                return;
            }

            // Predatory Strikes
            if(m_target->GetTypeId()==TYPEID_PLAYER && GetSpellProto()->SpellIconID == 1563)
            {
                ((Player*)m_target)->UpdateAttackPowerAndDamage();
                return;
            }
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Improved Aspect of the Viper
            if( GetId()==38390 && m_target->GetTypeId()==TYPEID_PLAYER )
            {
                if(apply)
                {
                    // + effect value for Aspect of the Viper
                    SpellModifier *mod = new SpellModifier;
                    mod->op = SPELLMOD_EFFECT1;
                    mod->value = m_amount;
                    mod->type = SPELLMOD_FLAT;
                    mod->spellId = GetId();
                    mod->mask[1] = 0x40000;

                    m_spellmod = mod;
                }

                ((Player*)m_target)->AddSpellMod(m_spellmod, apply);
                return;
            }
            // Glyph of Aspect of the Monkey
            if(m_spellProto->Id==56833)
            {
                if(apply)
                {
                    // Reduce backfire damage (dot damage) from Shadow Word: Death
                    SpellModifier *mod = new SpellModifier;
                    mod->op = SPELLMOD_CHANCE_OF_SUCCESS;
                    mod->value = 100;
                    mod->type = SPELLMOD_FLAT;
                    mod->spellId = GetId();
                    mod->mask[2] = 8192;
                    mod->mask[1] = 0x00000000;
                    mod->mask[0] = 524288;
                    m_spellmod = mod;
                }
                ((Player*)m_target)->AddSpellMod(m_spellmod, apply);
                return;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            // Improved Weapon Totems
            if( GetSpellProto()->SpellIconID == 57 && m_target->GetTypeId()==TYPEID_PLAYER )
            {
                if(apply)
                {
                    SpellModifier *mod = new SpellModifier;
                    mod->op = SPELLMOD_EFFECT1;
                    mod->value = m_amount;
                    mod->type = SPELLMOD_PCT;
                    mod->spellId = GetId();
                    switch (m_effIndex)
                    {
                        case 0:
                            mod->mask[1] = 0x002;    // Windfury Totem
                            break;
                        case 1:
                            mod->mask[1] = 0x004;    // Flametongue Totem
                            break;
                    }

                    m_spellmod = mod;
                }

                ((Player*)m_target)->AddSpellMod(m_spellmod, apply);
                return;
            }

            // Sentry Totem
            if (GetId() == 6495 && caster->GetTypeId() == TYPEID_PLAYER)
            {
                if (apply)
                {
                    uint64 guid = caster->m_SummonSlot[3];
                    if (guid)
                    {
                        Creature *totem = ObjectAccessor::GetCreature(*caster, guid);
                        if (totem && totem->isTotem())
                            ((Player*)caster)->CastSpell(totem, 6277, true);
                    }
                }
                else
                    ((Player*)caster)->StopCastingBindSight();
                return;
            }
            break;
        }
    }

    // pet auras
    if(PetAura const* petSpell = spellmgr.GetPetAura(GetId()))
    {
        if(apply)
            m_target->AddPetAura(petSpell);
        else
            m_target->RemovePetAura(petSpell);
        return;
    }

    if(GetEffIndex()==0 && m_target->GetTypeId()==TYPEID_PLAYER)
    {
        SpellAreaForAreaMapBounds saBounds = spellmgr.GetSpellAreaForAuraMapBounds(GetId());
        if(saBounds.first != saBounds.second)
        {
            uint32 zone, area;
            m_target->GetZoneAndAreaId(zone,area);

            for(SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
            {
                // some auras remove at aura remove
                if(!itr->second->IsFitToRequirements((Player*)m_target,zone,area))
                    m_target->RemoveAurasDueToSpell(itr->second->spellId);
                // some auras applied at aura apply
                else if(itr->second->autocast)
                {
                    if( !m_target->HasAuraEffect(itr->second->spellId,0) )
                        m_target->CastSpell(m_target,itr->second->spellId,true);
                }
            }
        }
    }
}

void AuraEffect::HandleAuraMounted(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    if(apply)
    {
        CreatureInfo const* ci = objmgr.GetCreatureTemplate(GetMiscValue());
        if(!ci)
        {
            sLog.outErrorDb("AuraMounted: `creature_template`='%u' not found in database (only need it modelid)",GetMiscValue());
            return;
        }

        uint32 team = 0;
        if (m_target->GetTypeId()==TYPEID_PLAYER)
            team = ((Player*)m_target)->GetTeam();

        uint32 display_id = objmgr.ChooseDisplayId(team,ci);
        CreatureModelInfo const *minfo = objmgr.GetCreatureModelRandomGender(display_id);
        if (minfo)
            display_id = minfo->modelid;

        m_target->Mount(display_id);
    }
    else
    {
        m_target->Unmount();
    }
}

void AuraEffect::HandleAuraWaterWalk(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_MOVE_WATER_WALK, 8+4);
    else
        data.Initialize(SMSG_MOVE_LAND_WALK, 8+4);
    data.append(m_target->GetPackGUID());
    data << uint32(0);
    m_target->SendMessageToSet(&data,true);
}

void AuraEffect::HandleAuraFeatherFall(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_MOVE_FEATHER_FALL, 8+4);
    else
        data.Initialize(SMSG_MOVE_NORMAL_FALL, 8+4);
    data.append(m_target->GetPackGUID());
    data << (uint32)0;
    m_target->SendMessageToSet(&data,true);
}

void AuraEffect::HandleAuraHover(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_MOVE_SET_HOVER, 8+4);
    else
        data.Initialize(SMSG_MOVE_UNSET_HOVER, 8+4);
    data.append(m_target->GetPackGUID());
    data << uint32(0);
    m_target->SendMessageToSet(&data,true);
}

void AuraEffect::HandleWaterBreathing(bool apply, bool Real)
{
    // update timers in client
    if(m_target->GetTypeId()==TYPEID_PLAYER)
        ((Player*)m_target)->UpdateMirrorTimers();
}

void AuraEffect::HandleAuraModShapeshift(bool apply, bool Real)
{
    if(!Real)
        return;

    uint32 modelid = 0;
    Powers PowerType = POWER_MANA;
    ShapeshiftForm form = ShapeshiftForm(GetMiscValue());
    switch(form)
    {
        case FORM_CAT:
            if(Player::TeamForRace(m_target->getRace())==ALLIANCE)
                modelid = 892;
            else
                modelid = 8571;
            PowerType = POWER_ENERGY;
            break;
        case FORM_TRAVEL:
            modelid = 632;
            break;
        case FORM_AQUA:
            if(Player::TeamForRace(m_target->getRace())==ALLIANCE)
                modelid = 2428;
            else
                modelid = 2428;
            break;
        case FORM_BEAR:
            if(Player::TeamForRace(m_target->getRace())==ALLIANCE)
                modelid = 2281;
            else
                modelid = 2289;
            PowerType = POWER_RAGE;
            break;
        case FORM_GHOUL:
            if(Player::TeamForRace(m_target->getRace())==ALLIANCE)
                modelid = 10045;
            break;
        case FORM_DIREBEAR:
            if(Player::TeamForRace(m_target->getRace())==ALLIANCE)
                modelid = 2281;
            else
                modelid = 2289;
            PowerType = POWER_RAGE;
            break;
        case FORM_CREATUREBEAR:
            modelid = 902;
            break;
        case FORM_GHOSTWOLF:
            modelid = 4613;
            break;
        case FORM_FLIGHT:
            if(Player::TeamForRace(m_target->getRace())==ALLIANCE)
                modelid = 20857;
            else
                modelid = 20872;
            break;
        case FORM_MOONKIN:
            if(Player::TeamForRace(m_target->getRace())==ALLIANCE)
                modelid = 15374;
            else
                modelid = 15375;
            break;
        case FORM_FLIGHT_EPIC:
            if(Player::TeamForRace(m_target->getRace())==ALLIANCE)
                modelid = 21243;
            else
                modelid = 21244;
            break;
        case FORM_METAMORPHOSIS:
            modelid = 25277;
            break;
        case FORM_AMBIENT:
        case FORM_SHADOW:
        case FORM_STEALTH:
            break;
        case FORM_TREE:
            modelid = 864;
            break;
        case FORM_BATTLESTANCE:
        case FORM_BERSERKERSTANCE:
        case FORM_DEFENSIVESTANCE:
            PowerType = POWER_RAGE;
            break;
        case FORM_SPIRITOFREDEMPTION:
            modelid = 16031;
            break;
        default:
            sLog.outError("Auras: Unknown Shapeshift Type: %u", GetMiscValue());
    }

    // remove polymorph before changing display id to keep new display id
    switch ( form )
    {
        case FORM_CAT:
        case FORM_TREE:
        case FORM_TRAVEL:
        case FORM_AQUA:
        case FORM_BEAR:
        case FORM_DIREBEAR:
        case FORM_FLIGHT_EPIC:
        case FORM_FLIGHT:
        case FORM_MOONKIN:
            // remove movement affects
            m_target->RemoveAurasByType(SPELL_AURA_MOD_ROOT);
            m_target->RemoveAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);

            // and polymorphic affects
            if(m_target->IsPolymorphed())
                m_target->RemoveAurasDueToSpell(m_target->getTransForm());
            break;
        default:
           break;
    }

    if(apply)
    {
        // remove other shapeshift before applying a new one
        if(m_target->m_ShapeShiftFormSpellId)
            m_target->RemoveAurasDueToSpell(m_target->m_ShapeShiftFormSpellId);

        m_target->SetByteValue(UNIT_FIELD_BYTES_2, 3, form);

        if(modelid > 0)
            m_target->SetDisplayId(modelid);

        if(PowerType != POWER_MANA)
        {
            // reset power to default values only at power change
            if(m_target->getPowerType()!=PowerType)
                m_target->setPowerType(PowerType);

            switch(form)
            {
                case FORM_CAT:
                case FORM_BEAR:
                case FORM_DIREBEAR:
                {
                    // get furor proc chance
                    uint32 FurorChance = 0;
                    Unit::AuraEffectList const& mDummy = m_target->GetAurasByType(SPELL_AURA_DUMMY);
                    for(Unit::AuraEffectList::const_iterator i = mDummy.begin(); i != mDummy.end(); ++i)
                    {
                        if ((*i)->GetSpellProto()->SpellIconID == 238)
                        {
                            FurorChance = (*i)->GetAmount();
                            break;
                        }
                    }

                    if (GetMiscValue() == FORM_CAT)
                    {
                        m_target->SetPower(POWER_ENERGY,0);
                        if(urand(1,100) <= FurorChance)
                            m_target->CastSpell(m_target,17099,true,NULL,this);
                    }
                    else
                    {
                        m_target->SetPower(POWER_RAGE,0);
                        if(urand(1,100) <= FurorChance)
                            m_target->CastSpell(m_target,17057,true,NULL,this);
                    }
                    break;
                }
                case FORM_BATTLESTANCE:
                case FORM_DEFENSIVESTANCE:
                case FORM_BERSERKERSTANCE:
                {
                    uint32 Rage_val = 0;
                    // Stance mastery + Tactical mastery (both passive, and last have aura only in defense stance, but need apply at any stance switch)
                    if(m_target->GetTypeId() == TYPEID_PLAYER)
                    {
                        PlayerSpellMap const& sp_list = ((Player *)m_target)->GetSpellMap();
                        for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
                        {
                            if(itr->second->state == PLAYERSPELL_REMOVED) continue;
                            SpellEntry const *spellInfo = sSpellStore.LookupEntry(itr->first);
                            if (spellInfo && spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR && spellInfo->SpellIconID == 139)
                                Rage_val += m_target->CalculateSpellDamage(spellInfo,0,spellInfo->EffectBasePoints[0],m_target) * 10;
                        }
                    }

                    if (m_target->GetPower(POWER_RAGE) > Rage_val)
                        m_target->SetPower(POWER_RAGE,Rage_val);
                    break;
                }
                default:
                    break;
            }
        }

        m_target->m_ShapeShiftFormSpellId = GetId();
        m_target->m_form = form;
    }
    else
    {
        if(modelid > 0)
            m_target->SetDisplayId(m_target->GetNativeDisplayId());
        m_target->SetByteValue(UNIT_FIELD_BYTES_2, 3, FORM_NONE);
        if(m_target->getClass() == CLASS_DRUID)
            m_target->setPowerType(POWER_MANA);
        m_target->m_ShapeShiftFormSpellId = 0;
        m_target->m_form = FORM_NONE;

        switch(form)
        {
            // Nordrassil Harness - bonus
            case FORM_BEAR:
            case FORM_DIREBEAR:
            case FORM_CAT:
            {
                if(AuraEffect* dummy = m_target->GetDummyAura(37315) )
                    m_target->CastSpell(m_target,37316,true,NULL,dummy);
                break;
            }
            // Nordrassil Regalia - bonus
            case FORM_MOONKIN:
            {
                if(AuraEffect* dummy = m_target->GetDummyAura(37324) )
                    m_target->CastSpell(m_target,37325,true,NULL,dummy);
                break;
            }
        }
    }

    // adding/removing linked auras
    // add/remove the shapeshift aura's boosts
    HandleShapeshiftBoosts(apply);

    if(m_target->GetTypeId()==TYPEID_PLAYER)
        ((Player*)m_target)->InitDataForForm();
}

void AuraEffect::HandleAuraTransform(bool apply, bool Real)
{
    if (apply)
    {
        // special case (spell specific functionality)
        if(GetMiscValue()==0)
        {
            // player applied only
            if(m_target->GetTypeId()!=TYPEID_PLAYER)
                return;

            switch(GetId())
            {
                // Orb of Deception
                case 16739:
                {
                    uint32 orb_model = m_target->GetNativeDisplayId();
                    switch(orb_model)
                    {
                        // Troll Female
                        case 1479: m_target->SetDisplayId(10134); break;
                        // Troll Male
                        case 1478: m_target->SetDisplayId(10135); break;
                        // Tauren Male
                        case 59:   m_target->SetDisplayId(10136); break;
                        // Human Male
                        case 49:   m_target->SetDisplayId(10137); break;
                        // Human Female
                        case 50:   m_target->SetDisplayId(10138); break;
                        // Orc Male
                        case 51:   m_target->SetDisplayId(10139); break;
                        // Orc Female
                        case 52:   m_target->SetDisplayId(10140); break;
                        // Dwarf Male
                        case 53:   m_target->SetDisplayId(10141); break;
                        // Dwarf Female
                        case 54:   m_target->SetDisplayId(10142); break;
                        // NightElf Male
                        case 55:   m_target->SetDisplayId(10143); break;
                        // NightElf Female
                        case 56:   m_target->SetDisplayId(10144); break;
                        // Undead Female
                        case 58:   m_target->SetDisplayId(10145); break;
                        // Undead Male
                        case 57:   m_target->SetDisplayId(10146); break;
                        // Tauren Female
                        case 60:   m_target->SetDisplayId(10147); break;
                        // Gnome Male
                        case 1563: m_target->SetDisplayId(10148); break;
                        // Gnome Female
                        case 1564: m_target->SetDisplayId(10149); break;
                        // BloodElf Female
                        case 15475: m_target->SetDisplayId(17830); break;
                        // BloodElf Male
                        case 15476: m_target->SetDisplayId(17829); break;
                        // Dranei Female
                        case 16126: m_target->SetDisplayId(17828); break;
                        // Dranei Male
                        case 16125: m_target->SetDisplayId(17827); break;
                        default: break;
                    }
                    break;
                }
                // Murloc costume
                case 42365: m_target->SetDisplayId(21723); break;
                default: break;
            }
        }
        else
        {
            CreatureInfo const * ci = objmgr.GetCreatureTemplate(GetMiscValue());
            if(!ci)
            {
                                                            //pig pink ^_^
                m_target->SetDisplayId(16358);
                sLog.outError("Auras: unknown creature id = %d (only need its modelid) Form Spell Aura Transform in Spell ID = %d", GetMiscValue(), GetId());
            }
            else
            {
                                                            // Will use the default model here
                if (uint32 modelid = ci->GetRandomValidModelId())
                    m_target->SetDisplayId(modelid);

                // Dragonmaw Illusion (set mount model also)
                if(GetId()==42016 && m_target->GetMountID() && !m_target->GetAurasByType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED).empty())
                    m_target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID,16314);
            }
            m_target->setTransForm(GetId());
        }

        // polymorph case
        if( Real && m_target->GetTypeId() == TYPEID_PLAYER && m_target->IsPolymorphed())
        {
            // for players, start regeneration after 1s (in polymorph fast regeneration case)
            // only if caster is Player (after patch 2.4.2)
            if(IS_PLAYER_GUID(GetCasterGUID()) )
                ((Player*)m_target)->setRegenTimer(1000);

            //dismount polymorphed target (after patch 2.4.2)
            if (m_target->IsMounted())
                m_target->RemoveAurasByType(SPELL_AURA_MOUNTED);
        }
    }
    else
    {
        Unit::AuraEffectList const& otherTransforms = m_target->GetAurasByType(SPELL_AURA_TRANSFORM);
        if(otherTransforms.empty())
        {
            m_target->SetDisplayId(m_target->GetNativeDisplayId());
            m_target->setTransForm(0);
        }
        else
        {
            // look for other transform auras
            AuraEffect* handledAura = *otherTransforms.begin();
            for(Unit::AuraEffectList::const_iterator i = otherTransforms.begin();i != otherTransforms.end(); ++i)
            {
                // negative auras are preferred
                if(!IsPositiveSpell((*i)->GetSpellProto()->Id))
                {
                    handledAura = *i;
                    break;
                }
            }
            handledAura->ApplyModifier(true);
        }

        // Dragonmaw Illusion (restore mount model)
        if(GetId()==42016 && m_target->GetMountID()==16314)
        {
            if(!m_target->GetAurasByType(SPELL_AURA_MOUNTED).empty())
            {
                uint32 cr_id = m_target->GetAurasByType(SPELL_AURA_MOUNTED).front()->GetMiscValue();
                if(CreatureInfo const* ci = objmgr.GetCreatureTemplate(cr_id))
                {
                    uint32 team = 0;
                    if (m_target->GetTypeId()==TYPEID_PLAYER)
                        team = ((Player*)m_target)->GetTeam();

                    uint32 display_id = objmgr.ChooseDisplayId(team,ci);
                    CreatureModelInfo const *minfo = objmgr.GetCreatureModelRandomGender(display_id);
                    if (minfo)
                        display_id = minfo->modelid;

                    m_target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID,display_id);
                }
            }
        }
    }
}

void AuraEffect::HandleForceReaction(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if(!Real)
        return;

    Player* player = (Player*)m_target;

    uint32 faction_id = GetMiscValue();
    uint32 faction_rank = m_amount;

    player->GetReputationMgr().ApplyForceReaction(faction_id,ReputationRank(faction_rank),apply);
    player->GetReputationMgr().SendForceReactions();
}

void AuraEffect::HandleAuraModSkill(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 prot=GetSpellProto()->EffectMiscValue[m_effIndex];
    int32 points = m_amount;

    ((Player*)m_target)->ModifySkillBonus(prot,(apply ? points: -points),m_auraName==SPELL_AURA_MOD_SKILL_TALENT);
    if(prot == SKILL_DEFENSE)
        ((Player*)m_target)->UpdateDefenseBonusesMod();
}

void AuraEffect::HandleChannelDeathItem(bool apply, bool Real)
{
    if(Real && !apply)
    {
        Unit* caster = GetCaster();
        Unit* victim = m_target;
        if(!caster || caster->GetTypeId() != TYPEID_PLAYER || !victim)// || m_removeMode!=AURA_REMOVE_BY_DEATH)
            return;

        //we cannot check removemode = death
        //talent will remove the caster's aura->interrupt channel->remove victim aura
        if(victim->GetHealth() > 0)
            return;
        // Item amount
        if (m_amount <= 0)
            return;

        SpellEntry const *spellInfo = GetSpellProto();
        if(spellInfo->EffectItemType[m_effIndex] == 0)
            return;

        // Soul Shard only from non-grey units
        if( spellInfo->EffectItemType[m_effIndex] == 6265 &&
            (victim->getLevel() <= Trinity::XP::GetGrayLevel(caster->getLevel()) ||
             victim->GetTypeId()==TYPEID_UNIT && !((Player*)caster)->isAllowedToLoot((Creature*)victim)) )
            return;
        //Adding items
        uint32 noSpaceForCount = 0;
        uint32 count = m_amount;

        ItemPosCountVec dest;
        uint8 msg = ((Player*)caster)->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, spellInfo->EffectItemType[m_effIndex], count, &noSpaceForCount);
        if( msg != EQUIP_ERR_OK )
        {
            count-=noSpaceForCount;
            ((Player*)caster)->SendEquipError( msg, NULL, NULL );
            if (count==0)
                return;
        }

        Item* newitem = ((Player*)caster)->StoreNewItem(dest, spellInfo->EffectItemType[m_effIndex], true);
        ((Player*)caster)->SendNewItem(newitem, count, true, false);
    }
}

void AuraEffect::HandleBindSight(bool apply, bool Real)
{
    Unit* caster = GetCaster();
    if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)caster)->SetViewpoint(m_target, apply);
}

void AuraEffect::HandleFarSight(bool apply, bool Real)
{
    //Handled by client
}

void AuraEffect::HandleAuraTrackCreatures(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    m_target->SetUInt32Value(PLAYER_TRACK_CREATURES, apply ? ((uint32)1)<<(GetMiscValue()-1) : 0 );
}

void AuraEffect::HandleAuraTrackResources(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    m_target->SetUInt32Value(PLAYER_TRACK_RESOURCES, apply ? ((uint32)1)<<(GetMiscValue()-1): 0 );
}

void AuraEffect::HandleAuraTrackStealthed(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    m_target->ApplyModFlag(PLAYER_FIELD_BYTES,PLAYER_FIELD_BYTE_TRACK_STEALTHED,apply);
}

void AuraEffect::HandleAuraModScale(bool apply, bool Real)
{
    m_target->ApplyPercentModFloatValue(OBJECT_FIELD_SCALE_X,m_amount,apply);
}

/*void AuraEffect::HandleModPossess(bool apply, bool Real)
{
    if(!Real)
        return;

    if(m_target->getLevel() > m_amount)
        return;

    // not possess yourself
    if(GetCasterGUID() == m_target->GetGUID())
        return;

    Unit* caster = GetCaster();
    if(!caster)
        return;

    if( apply )
    {
        m_target->SetCharmerGUID(GetCasterGUID());
        m_target->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,caster->getFaction());

        caster->SetCharm(m_target);

        if(caster->GetTypeId() == TYPEID_PLAYER)
        {
            ((Player*)caster)->SetFarSightGUID(m_target->GetGUID());
            ((Player*)caster)->SetClientControl(m_target, 1);
        }

        m_target->CombatStop();
        m_target->DeleteThreatList();
        if(m_target->GetTypeId() == TYPEID_UNIT)
        {
            m_target->StopMoving();
            m_target->GetMotionMaster()->Clear();
            m_target->GetMotionMaster()->MoveIdle();
        }
        else if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            ((Player*)m_target)->SetClientControl(m_target, 0);
        }

        if(CharmInfo *charmInfo = m_target->InitCharmInfo(m_target))
            charmInfo->InitPossessCreateSpells();

        if(caster->GetTypeId() == TYPEID_PLAYER)
            ((Player*)caster)->PossessSpellInitialize();
    }
    else
    {
        m_target->SetCharmerGUID(0);
        caster->InterruptSpell(CURRENT_CHANNELED_SPELL);    // the spell is not automatically canceled when interrupted, do it now

        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            ((Player*)m_target)->setFactionForRace(m_target->getRace());
            ((Player*)m_target)->SetClientControl(m_target, 1);
        }
        else if(m_target->GetTypeId() == TYPEID_UNIT)
        {
            CreatureInfo const *cinfo = ((Creature*)m_target)->GetCreatureInfo();
            m_target->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,cinfo->faction_A);
        }

        caster->SetCharm(NULL);

        if(caster->GetTypeId() == TYPEID_PLAYER)
        {
            ((Player*)caster)->SetFarSightGUID(0);
            ((Player*)caster)->SetClientControl(m_target,0);

            WorldPacket data(SMSG_PET_SPELLS, 8+4);
            data << uint64(0);
            data << uint32(0);
            ((Player*)caster)->GetSession()->SendPacket(&data);
        }

        if(m_target->GetTypeId() == TYPEID_UNIT)
        {
            ((Creature*)m_target)->AIM_Initialize();

            if (((Creature*)m_target)->AI())
                ((Creature*)m_target)->AI()->AttackStart(caster);
        }
    }
}

void AuraEffect::HandleModPossessPet(bool apply, bool Real)
{
    if(!Real)
        return;

    Unit* caster = GetCaster();
    if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Pet *pet = caster->GetGuardianPet();
    if(!pet || pet != m_target)
        return;

    if(apply)
        pet->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_24);
    else
        pet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_24);

    ((Player*)caster)->SetFarSightGUID(apply ? pet->GetGUID() : 0);
    ((Player*)caster)->SetCharm(apply ? pet : NULL);
    ((Player*)caster)->SetClientControl(pet, apply ? 1 : 0);

    if(apply)
    {
        pet->StopMoving();
        pet->GetMotionMaster()->Clear();
        pet->GetMotionMaster()->MoveIdle();
    }
    else
    {
        pet->AttackStop();
        pet->GetMotionMaster()->MoveFollow(caster, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
        pet->SetUnitMovementFlags(MOVEMENTFLAG_NONE);
    }
}*/

void AuraEffect::HandleAuraModPetTalentsPoints(bool Apply, bool Real)
{
    if(!Real)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate pet tlaent points
    if (Pet *pet = ((Player*)m_target)->GetPet())
        pet->InitTalentForLevel();
}

/*void AuraEffect::HandleModCharm(bool apply, bool Real)
{
    if(!Real)
        return;

    // not charm yourself
    if(GetCasterGUID() == m_target->GetGUID())
        return;

    Unit* caster = GetCaster();
    if(!caster)
        return;

    if(int32(m_target->getLevel()) <= m_amount)
    {
        if( apply )
        {
            m_target->SetCharmerGUID(GetCasterGUID());
            m_target->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,caster->getFaction());
            m_target->CastStop(m_target==caster ? GetId() : 0);
            caster->SetCharm(m_target);

            m_target->CombatStop();
            m_target->DeleteThreatList();

            if(m_target->GetTypeId() == TYPEID_UNIT)
            {
                ((Creature*)m_target)->AIM_Initialize();
                CharmInfo *charmInfo = m_target->InitCharmInfo(m_target);
                charmInfo->InitCharmCreateSpells();
                charmInfo->SetReactState( REACT_DEFENSIVE );

                if(caster->GetTypeId() == TYPEID_PLAYER && caster->getClass() == CLASS_WARLOCK)
                {
                    CreatureInfo const *cinfo = ((Creature*)m_target)->GetCreatureInfo();
                    if(cinfo && cinfo->type == CREATURE_TYPE_DEMON)
                    {
                        //to prevent client crash
                        m_target->SetFlag(UNIT_FIELD_BYTES_0, 2048);
                        //just to enable stat window
                        charmInfo->SetPetNumber(objmgr.GeneratePetNumber(), true);
                        //if charmed two demons the same session, the 2nd gets the 1st one's name
                        m_target->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, time(NULL));
                    }
                }
            }

            if(caster->GetTypeId() == TYPEID_PLAYER)
            {
                ((Player*)caster)->CharmSpellInitialize();
            }
        }
        else
        {
            m_target->SetCharmerGUID(0);

            if(m_target->GetTypeId() == TYPEID_PLAYER)
                ((Player*)m_target)->setFactionForRace(m_target->getRace());
            else
            {
                CreatureInfo const *cinfo = ((Creature*)m_target)->GetCreatureInfo();

                // restore faction
                if(((Creature*)m_target)->isPet())
                {
                    if(Unit* owner = m_target->GetOwner())
                        m_target->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,owner->getFaction());
                    else if(cinfo)
                        m_target->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,cinfo->faction_A);
                }
                else if(cinfo)                              // normal creature
                    m_target->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,cinfo->faction_A);

                // restore UNIT_FIELD_BYTES_0
                if(cinfo && caster->GetTypeId() == TYPEID_PLAYER && caster->getClass() == CLASS_WARLOCK && cinfo->type == CREATURE_TYPE_DEMON)
                {
                    CreatureDataAddon const *cainfo = ((Creature*)m_target)->GetCreatureAddon();
                    if(cainfo && cainfo->bytes0 != 0)
                        m_target->SetUInt32Value(UNIT_FIELD_BYTES_0, cainfo->bytes0);
                    else
                        m_target->RemoveFlag(UNIT_FIELD_BYTES_0, 2048);

                    if(m_target->GetCharmInfo())
                        m_target->GetCharmInfo()->SetPetNumber(0, true);
                    else
                        sLog.outError("AuraEffect::HandleModCharm: target="I64FMTD" with typeid=%d has a charm aura but no charm info!", m_target->GetGUID(), m_target->GetTypeId());
                }
            }

            caster->SetCharm(NULL);

            if(caster->GetTypeId() == TYPEID_PLAYER)
            {
                WorldPacket data(SMSG_PET_SPELLS, 8+4);
                data << uint64(0);
                data << uint32(0);
                ((Player*)caster)->GetSession()->SendPacket(&data);
            }
            if(m_target->GetTypeId() == TYPEID_UNIT)
            {
                ((Creature*)m_target)->AIM_Initialize();
                if (((Creature*)m_target)->AI())
                    ((Creature*)m_target)->AI()->AttackStart(caster);
            }
        }
    }
}*/

void AuraEffect::HandleModConfuse(bool apply, bool Real)
{
    if(!Real)
        return;

    //m_target->SetConfused(apply, GetCasterGUID(), GetId());
    m_target->SetControlled(apply, UNIT_STAT_CONFUSED);
}

void AuraEffect::HandleModFear(bool apply, bool Real)
{
    if (!Real)
        return;

    //m_target->SetFeared(apply, GetCasterGUID(), GetId());
    m_target->SetControlled(apply, UNIT_STAT_FLEEING);
}

void AuraEffect::HandleFeignDeath(bool apply, bool Real)
{
    if(!Real)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if( apply )
    {
        /*
        WorldPacket data(SMSG_FEIGN_DEATH_RESISTED, 9);
        data<<m_target->GetGUID();
        data<<uint8(0);
        m_target->SendMessageToSet(&data,true);
        */

        std::list<Unit*> targets;
        Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(m_target, m_target, World::GetMaxVisibleDistance());
        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(m_target, targets, u_check);
        m_target->VisitNearbyObject(World::GetMaxVisibleDistance(), searcher);
        for(std::list<Unit*>::iterator iter = targets.begin(); iter != targets.end(); ++iter)
        {
            if(!(*iter)->hasUnitState(UNIT_STAT_CASTING))
                continue;

            for(uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
            {
                if((*iter)->m_currentSpells[i]
                && (*iter)->m_currentSpells[i]->m_targets.getUnitTargetGUID() == m_target->GetGUID())
                {
                    (*iter)->InterruptSpell(i, false);
                }
            }
        }
                                                            // blizz like 2.0.x
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_29);
                                                            // blizz like 2.0.x
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                                                            // blizz like 2.0.x
        m_target->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);

        m_target->addUnitState(UNIT_STAT_DIED);
        m_target->CombatStop();
        m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

        // prevent interrupt message
        if(GetCasterGUID()==m_target->GetGUID() && m_target->m_currentSpells[CURRENT_GENERIC_SPELL])
            m_target->m_currentSpells[CURRENT_GENERIC_SPELL]->finish();
        m_target->InterruptNonMeleeSpells(true);
        m_target->getHostilRefManager().deleteReferences();
    }
    else
    {
        /*
        WorldPacket data(SMSG_FEIGN_DEATH_RESISTED, 9);
        data<<m_target->GetGUID();
        data<<uint8(1);
        m_target->SendMessageToSet(&data,true);
        */
                                                            // blizz like 2.0.x
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_29);
                                                            // blizz like 2.0.x
        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                                                            // blizz like 2.0.x
        m_target->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);

        m_target->clearUnitState(UNIT_STAT_DIED);
    }
}

void AuraEffect::HandleAuraModDisarm(bool apply, bool Real)
{
    if (!Real)
        return;
    AuraType type = AuraType(GetAuraName());

    //Prevent handling aura twice
    if(apply && m_target->GetAurasByType(type).size()>1)
        return;
    if(!apply && m_target->HasAuraType(type))
        return;
    uint32 field, flag, slot;
    WeaponAttackType attType;
    switch (type)
    {
    case SPELL_AURA_MOD_DISARM:
        field=UNIT_FIELD_FLAGS;
        flag=UNIT_FLAG_DISARMED;
        slot=EQUIPMENT_SLOT_MAINHAND;
        attType=BASE_ATTACK;
    break;
    case SPELL_AURA_MOD_DISARM_OFFHAND:
        field=UNIT_FIELD_FLAGS_2;
        flag=UNIT_FLAG2_DISARM_OFFHAND;
        slot=EQUIPMENT_SLOT_OFFHAND;
        attType=OFF_ATTACK;
    break;
    case SPELL_AURA_MOD_DISARM_RANGED:
        field=UNIT_FIELD_FLAGS_2;
        flag=UNIT_FLAG2_DISARM_RANGED;
        slot=EQUIPMENT_SLOT_RANGED;
        attType=RANGED_ATTACK;
    break;
    }
    if(apply)
        m_target->SetFlag(field, flag);
    else
        m_target->RemoveFlag(field, flag);

    if (m_target->GetTypeId() == TYPEID_PLAYER)
    {
        Item *pItem = ((Player*)m_target)->GetItemByPos( INVENTORY_SLOT_BAG_0, slot );
        if(!pItem )
            return;
        ((Player*)m_target)->_ApplyItemMods(pItem, slot, !apply);
    }
    else if (((Creature*)m_target)->GetCurrentEquipmentId())
        m_target->UpdateDamagePhysical(attType);
}

void AuraEffect::HandleModStealth(bool apply, bool Real)
{
    if(!Real)
        return;

    if(apply)
    {
        // drop flag at stealth in bg
        m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

        m_target->SetStandFlags(UNIT_STAND_FLAGS_CREEP);
        if(m_target->GetTypeId()==TYPEID_PLAYER)
            m_target->SetFlag(PLAYER_FIELD_BYTES2, 0x2000);

        // apply only if not in GM invisibility (and overwrite invisibility state)
        if(m_target->GetVisibility() != VISIBILITY_OFF)
            m_target->SetVisibility(VISIBILITY_GROUP_STEALTH);
    }
    else if(!m_target->HasAuraType(SPELL_AURA_MOD_STEALTH)) // if last SPELL_AURA_MOD_STEALTH
    {
        m_target->RemoveStandFlags(UNIT_STAND_FLAGS_CREEP);
        if(m_target->GetTypeId()==TYPEID_PLAYER)
            m_target->RemoveFlag(PLAYER_FIELD_BYTES2, 0x2000);

        if(m_target->GetVisibility() != VISIBILITY_OFF)
            m_target->SetVisibility(VISIBILITY_ON);
    }
}

void AuraEffect::HandleInvisibility(bool apply, bool Real)
{
    if(apply)
    {
        m_target->m_invisibilityMask |= (1 << GetMiscValue());

        if(Real)
        {
            m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

            // apply glow vision
            if(m_target->GetTypeId()==TYPEID_PLAYER)
                m_target->SetFlag(PLAYER_FIELD_BYTES2,PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW);

            m_target->SetToNotify();
        }
    }
    else
    {
        // recalculate value at modifier remove (current aura already removed)
        m_target->m_invisibilityMask = 0;
        Unit::AuraEffectList const& auras = m_target->GetAurasByType(SPELL_AURA_MOD_INVISIBILITY);
        for(Unit::AuraEffectList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
            m_target->m_invisibilityMask |= (1 << GetMiscValue());

        // only at real aura remove and if not have different invisibility auras.
        if(Real)
        {
            // remove glow vision
            if(!m_target->m_invisibilityMask && m_target->GetTypeId() == TYPEID_PLAYER)
                m_target->RemoveFlag(PLAYER_FIELD_BYTES2,PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW);

            m_target->SetToNotify();
        }
    }
}

void AuraEffect::HandleInvisibilityDetect(bool apply, bool Real)
{
    if(apply)
    {
        m_target->m_detectInvisibilityMask |= (1 << GetMiscValue());
    }
    else
    {
        // recalculate value at modifier remove (current aura already removed)
        m_target->m_detectInvisibilityMask = 0;
        Unit::AuraEffectList const& auras = m_target->GetAurasByType(SPELL_AURA_MOD_INVISIBILITY_DETECTION);
        for(Unit::AuraEffectList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
            m_target->m_detectInvisibilityMask |= (1 << GetMiscValue());
    }
    if(Real && m_target->GetTypeId()==TYPEID_PLAYER)
        //ObjectAccessor::UpdateVisibilityForPlayer((Player*)m_target);
        m_target->SetToNotify();
}

void AuraEffect::HandleAuraModSilence(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    if(apply)
    {
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
        // Stop cast only spells vs PreventionType == SPELL_PREVENTION_TYPE_SILENCE
        for (uint32 i = CURRENT_MELEE_SPELL; i < CURRENT_MAX_SPELL;i++)
        {
            Spell* currentSpell = m_target->m_currentSpells[i];
            if (currentSpell && currentSpell->m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
            {
                uint32 state = currentSpell->getState();
                // Stop spells on prepare or casting state
                if ( state == SPELL_STATE_PREPARING || state == SPELL_STATE_CASTING )
                {
                    currentSpell->cancel();
                }
            }
        }

        switch (GetId())
        {
            // Arcane Torrent (Energy)
            case 25046:
            {
                Unit * caster = GetCaster();
                if (!caster)
                    return;

                // Search Mana Tap auras on caster
                AuraEffect * dummy = caster->GetDummyAura(28734);
                if (dummy)
                {
                    int32 bp = dummy->GetParentAura()->GetStackAmount() * 10;
                    caster->CastCustomSpell(caster, 25048, &bp, NULL, NULL, true);
                    m_target->RemoveAurasDueToSpell(28734);
                }
            }
        }
    }
    else
    {
        // Real remove called after current aura remove from lists, check if other similar auras active
        if(m_target->HasAuraType(SPELL_AURA_MOD_SILENCE))
            return;

        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
    }
}

void AuraEffect::HandleModThreat(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    if(!m_target->isAlive())
        return;

    Unit* caster = GetCaster();

    if(!caster || !caster->isAlive())
        return;

    int level_diff = 0;
    int multiplier = 0;
    switch (GetId())
    {
        // Arcane Shroud
        case 26400:
            level_diff = m_target->getLevel() - 60;
            multiplier = 2;
            break;
        // The Eye of Diminution
        case 28862:
            level_diff = m_target->getLevel() - 60;
            multiplier = 1;
            break;
    }
    if (level_diff > 0)
        m_amount += multiplier * level_diff;

    for(int8 x=0;x < MAX_SPELL_SCHOOL;x++)
    {
        if(GetMiscValue() & int32(1<<x))
        {
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                ApplyPercentModFloatVar(m_target->m_threatModifier[x], GetParentAura()->IsPositive() ? m_amount : -m_amount, apply);
        }
    }
}

void AuraEffect::HandleAuraModTotalThreat(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    if(!m_target->isAlive() || m_target->GetTypeId()!= TYPEID_PLAYER)
        return;

    Unit* caster = GetCaster();

    if(!caster || !caster->isAlive())
        return;

    float threatMod = 0.0f;
    if(apply)
        threatMod = float(m_amount);
    else
        threatMod =  float(-m_amount);

    m_target->getHostilRefManager().threatAssist(caster, threatMod);
}

void AuraEffect::HandleModTaunt(bool apply, bool Real)
{
    // only at real add/remove aura
    if(!Real)
        return;

    if(!m_target->isAlive() || !m_target->CanHaveThreatList())
        return;

    Unit* caster = GetCaster();

    if(!caster || !caster->isAlive() || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if(apply)
        m_target->TauntApply(caster);
    else
    {
        // When taunt aura fades out, mob will switch to previous target if current has less than 1.1 * secondthreat
        m_target->TauntFadeOut(caster);
    }
}

/*********************************************************/
/***                  MODIFY SPEED                     ***/
/*********************************************************/
void AuraEffect::HandleAuraModIncreaseSpeed(bool /*apply*/, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    m_target->UpdateSpeed(MOVE_RUN, true);
}

void AuraEffect::HandleAuraModIncreaseMountedSpeed(bool /*apply*/, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    m_target->UpdateSpeed(MOVE_RUN, true);
}

void AuraEffect::HandleAuraModIncreaseFlightSpeed(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    // Enable Fly mode for flying mounts
    if (m_auraName == SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED)
    {
        WorldPacket data;
        if(apply)
            data.Initialize(SMSG_MOVE_SET_CAN_FLY, 12);
        else
            data.Initialize(SMSG_MOVE_UNSET_CAN_FLY, 12);
        data.append(m_target->GetPackGUID());
        data << uint32(0);                                      // unknown
        m_target->SendMessageToSet(&data, true);

        //Players on flying mounts must be immune to polymorph
        if (m_target->GetTypeId()==TYPEID_PLAYER)
            m_target->ApplySpellImmune(GetId(),IMMUNITY_MECHANIC,1<<MECHANIC_POLYMORPH,apply);

        // Dragonmaw Illusion (overwrite mount model, mounted aura already applied)
        if( apply && m_target->HasAuraEffect(42016,0) && m_target->GetMountID())
            m_target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID,16314);
    }

    m_target->UpdateSpeed(MOVE_FLIGHT, true);
}

void AuraEffect::HandleAuraModIncreaseSwimSpeed(bool /*apply*/, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    m_target->UpdateSpeed(MOVE_SWIM, true);
}

void AuraEffect::HandleAuraModDecreaseSpeed(bool /*apply*/, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    m_target->UpdateSpeed(MOVE_RUN, true);
    m_target->UpdateSpeed(MOVE_SWIM, true);
    m_target->UpdateSpeed(MOVE_FLIGHT, true);
}

void AuraEffect::HandleAuraModUseNormalSpeed(bool /*apply*/, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    m_target->UpdateSpeed(MOVE_RUN,  true);
    m_target->UpdateSpeed(MOVE_SWIM, true);
    m_target->UpdateSpeed(MOVE_FLIGHT,  true);
}

/*********************************************************/
/***                     IMMUNITY                      ***/
/*********************************************************/

void AuraEffect::HandleModMechanicImmunity(bool apply, bool Real)
{
    uint32 mechanic;
    if (GetSpellProto()->EffectApplyAuraName[GetEffIndex()]==SPELL_AURA_MECHANIC_IMMUNITY)
        mechanic = 1 << GetMiscValue();
    else //SPELL_AURA_MECHANIC_IMMUNITY_MASK
        mechanic = GetMiscValue();
    //immune movement impairment and loss of control
    if(GetId()==42292 || GetId()==59752)
        mechanic=IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK;

    if(apply && GetSpellProto()->AttributesEx & SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY)
    {
        Unit::AuraMap& Auras = m_target->GetAuras();
        for(Unit::AuraMap::iterator iter = Auras.begin(); iter != Auras.end();)
        {
            SpellEntry const *spell = iter->second->GetSpellProto();
            if (!( spell->Attributes & SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY) && // spells unaffected by invulnerability
                   spell->Id != GetId())
            {
                //check for mechanic mask
                if(GetAllSpellMechanicMask(spell) & mechanic)
                {
                    m_target->RemoveAura(iter);
                }
                else
                    ++iter;
            }
            else
                ++iter;
        }
    }

    // Patch 3.0.3 Bladestorm now breaks all snares and roots on the warrior when activated.
    // however not all mechanic specified in immunity
    if (apply && GetId()==46924)
    {
        m_target->RemoveAurasByType(SPELL_AURA_MOD_ROOT);
        m_target->RemoveAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
    }

    m_target->ApplySpellImmune(GetId(),IMMUNITY_MECHANIC,mechanic,apply);

    // Bestial Wrath
    if ( GetSpellProto()->SpellFamilyName == SPELLFAMILY_HUNTER && GetSpellProto()->Id == 19574)
    {
        // The Beast Within cast on owner if talent present
        if ( Unit* owner = m_target->GetOwner() )
        {
            // Search talent
            Unit::AuraEffectList const& m_dummyAuras = owner->GetAurasByType(SPELL_AURA_DUMMY);
            for(Unit::AuraEffectList::const_iterator i = m_dummyAuras.begin(); i != m_dummyAuras.end(); ++i)
            {
                if ( (*i)->GetSpellProto()->SpellIconID == 2229 )
                {
                    if (apply)
                        owner->CastSpell(owner, 34471, true, 0, this);
                    else
                        owner->RemoveAurasDueToSpell(34471);
                    break;
                }
            }
        }
    }

    // The Beast Within and Bestial Wrath - immunity
    if(GetId() == 19574 || GetId() == 34471)
    {
        if(apply)
        {
            m_target->CastSpell(m_target,24395,true);
            m_target->CastSpell(m_target,24396,true);
            m_target->CastSpell(m_target,24397,true);
            m_target->CastSpell(m_target,26592,true);
        }
        else
        {
            m_target->RemoveAurasDueToSpell(24395);
            m_target->RemoveAurasDueToSpell(24396);
            m_target->RemoveAurasDueToSpell(24397);
            m_target->RemoveAurasDueToSpell(26592);
        }
    }

    // Heroic Fury (remove Intercept cooldown)
    if( apply && GetId() == 60970 && m_target->GetTypeId() == TYPEID_PLAYER )
    {
        ((Player*)m_target)->RemoveSpellCooldown(20252);

        WorldPacket data(SMSG_CLEAR_COOLDOWN, (4+8));
        data << uint32(20252);
        data << uint64(m_target->GetGUID());
        ((Player*)m_target)->GetSession()->SendPacket(&data);
    }
}

//this method is called whenever we add / remove aura which gives m_target some imunity to some spell effect
void AuraEffect::HandleAuraModEffectImmunity(bool apply, bool Real)
{
    // when removing flag aura, handle flag drop
    if( !apply && m_target->GetTypeId() == TYPEID_PLAYER
        && (GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION) )
    {
        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            if(((Player*)m_target)->InBattleGround())
            {
                if( BattleGround *bg = ((Player*)m_target)->GetBattleGround() )
                    bg->EventPlayerDroppedFlag(((Player*)m_target));
            }
            else
                sOutdoorPvPMgr.HandleDropFlag((Player*)m_target,GetSpellProto()->Id);
        }
    }

    m_target->ApplySpellImmune(GetId(),IMMUNITY_EFFECT,GetMiscValue(),apply);
}

void AuraEffect::HandleAuraModStateImmunity(bool apply, bool Real)
{
    if(apply && Real && GetSpellProto()->AttributesEx & SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY)
    {
        m_target->RemoveAurasByType(AuraType(GetMiscValue()), NULL , GetParentAura());
    }

    m_target->ApplySpellImmune(GetId(),IMMUNITY_STATE,GetMiscValue(),apply);
}

void AuraEffect::HandleAuraModSchoolImmunity(bool apply, bool Real)
{
    if(apply && GetMiscValue() == SPELL_SCHOOL_MASK_NORMAL)
        m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

    m_target->ApplySpellImmune(GetId(),IMMUNITY_SCHOOL,GetMiscValue(),apply);

    // remove all flag auras (they are positive, but they must be removed when you are immune)
    if( this->GetSpellProto()->AttributesEx & SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY
        && this->GetSpellProto()->AttributesEx2 & SPELL_ATTR_EX2_DAMAGE_REDUCED_SHIELD )
        m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

    // TODO: optimalize this cycle - use RemoveAurasWithInterruptFlags call or something else
    if( Real && apply
        && GetSpellProto()->AttributesEx & SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY
        && IsPositiveSpell(GetId()) )                       //Only positive immunity removes auras
    {
        uint32 school_mask = GetMiscValue();
        Unit::AuraMap& Auras = m_target->GetAuras();
        for(Unit::AuraMap::iterator iter = Auras.begin(); iter != Auras.end();)
        {
            SpellEntry const *spell = iter->second->GetSpellProto();
            if((GetSpellSchoolMask(spell) & school_mask)//Check for school mask
                && !( spell->Attributes & SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY)   //Spells unaffected by invulnerability
                && !iter->second->IsPositive()          //Don't remove positive spells
                && spell->Id != GetId() )               //Don't remove self
            {
                m_target->RemoveAura(iter);
            }
            else
                ++iter;
        }
    }
    if( Real && GetSpellProto()->Mechanic == MECHANIC_BANISH )
    {
        if( apply )
            m_target->addUnitState(UNIT_STAT_ISOLATED);
        else
            m_target->clearUnitState(UNIT_STAT_ISOLATED);
    }
}

void AuraEffect::HandleAuraModDmgImmunity(bool apply, bool Real)
{
    m_target->ApplySpellImmune(GetId(),IMMUNITY_DAMAGE,GetMiscValue(),apply);
}

void AuraEffect::HandleAuraModDispelImmunity(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    m_target->ApplySpellDispelImmunity(m_spellProto, DispelType(GetMiscValue()), apply);
}

void AuraEffect::HandleAuraProcTriggerSpell(bool apply, bool Real)
{
    if(!Real)
        return;
}

void AuraEffect::HandleAuraModStalked(bool apply, bool Real)
{
    // used by spells: Hunter's Mark, Mind Vision, Syndicate Tracker (MURP) DND
    if(apply)
        m_target->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
    else
        m_target->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
}

/*********************************************************/
/***                   PERIODIC                        ***/
/*********************************************************/

void AuraEffect::HandlePeriodicTriggerSpell(bool apply, bool Real)
{
    m_isPeriodic = apply;
    if (m_spellProto->Id == 66 && !apply)
    {
        if (GetParentAura()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
            m_target->CastSpell(m_target, 32612, true, NULL, this);
    }
}

void AuraEffect::HandlePeriodicTriggerSpellWithValue(bool apply, bool Real)
{
    m_isPeriodic = apply;

    // Wrath of the Astromancer
    if(!apply && m_spellProto->Id == 42783)
    {
        m_target->CastSpell(m_target, 42787, true, 0, this);
    }
}

void AuraEffect::HandlePeriodicEnergize(bool apply, bool Real)
{
    if (!Real)
        return;

    m_isPeriodic = apply;

    // Replenishment (0.25% from max)
    // Infinite Replenishment
    if (m_spellProto->SpellIconID == 3184 && m_spellProto->SpellVisual[0] == 12495)
        m_amount = m_target->GetMaxPower(POWER_MANA) * 25 / 10000;
}

void AuraEffect::HandleAuraPowerBurn(bool apply, bool Real)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandleAuraPeriodicDummy(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    // For prevent double apply bonuses
    bool loading = (m_target->GetTypeId() == TYPEID_PLAYER && ((Player*)m_target)->GetSession()->PlayerLoading());

    Unit* caster = GetCaster();

    SpellEntry const*spell = GetSpellProto();
    switch( spell->SpellFamilyName)
    {
        case SPELLFAMILY_HUNTER:
        {
            // Explosive Shot
            if (apply && !loading && caster)
                m_amount += int32(caster->GetTotalAttackPowerValue(RANGED_ATTACK) * 16 / 100);
            break;
        }
    }

    m_isPeriodic = apply;
}

void AuraEffect::HandlePeriodicHeal(bool apply, bool Real)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandlePeriodicDamage(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    m_isPeriodic = apply;

    // For prevent double apply bonuses
    bool loading = (m_target->GetTypeId() == TYPEID_PLAYER && ((Player*)m_target)->GetSession()->PlayerLoading());

    // Custom damage calculation after
    if (!apply || loading)
        return;

    Unit *caster = GetCaster();
    if (!caster)
        return;

    switch (m_spellProto->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            // Pounce Bleed
            if ( m_spellProto->SpellIconID == 147 && m_spellProto->SpellVisual[0] == 0 )
            {
                // $AP*0.18/6 bonus per tick
                m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 3 / 100);
                return;
            }
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            // Rend
            if (m_spellProto->SpellFamilyFlags[0] & 0x20)
            {
                // $0.2*(($MWB+$mwb)/2+$AP/14*$MWS) bonus per tick
                float ap = caster->GetTotalAttackPowerValue(BASE_ATTACK);
                int32 mws = caster->GetAttackTime(BASE_ATTACK);
                float mwb_min = caster->GetWeaponDamageRange(BASE_ATTACK,MINDAMAGE);
                float mwb_max = caster->GetWeaponDamageRange(BASE_ATTACK,MAXDAMAGE);
                m_amount+=int32(((mwb_min+mwb_max)/2+ap*mws/14000)*0.2f);
                return;
            }
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            // Rake
            if (m_spellProto->SpellFamilyFlags[0] & 0x1000)
            {
                // $AP*0.06 bonus per tick
                m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 6 / 100);
                return;
            }
            // Lacerate
            if (m_spellProto->SpellFamilyFlags[1] & 0x0000000100)
            {
                // $AP*0.05/5 bonus per tick
                m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) / 100);
                return;
            }
            // Rip
            if (m_spellProto->SpellFamilyFlags[1] & 0x800000)
            {
                // 0.01*$AP*cp
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                uint8 cp = ((Player*)caster)->GetComboPoints();

                // Idol of Feral Shadows. Cant be handled as SpellMod in SpellAura:Dummy due its dependency from CPs
                Unit::AuraEffectList const& dummyAuras = caster->GetAurasByType(SPELL_AURA_DUMMY);
                for(Unit::AuraEffectList::const_iterator itr = dummyAuras.begin(); itr != dummyAuras.end(); ++itr)
                {
                    if((*itr)->GetId()==34241)
                    {
                        m_amount += cp * (*itr)->GetAmount();
                        break;
                    }
                }
                m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * cp / 100);
                return;
            }
            // Lock Jaw
            if (m_spellProto->SpellFamilyFlags[1] & 0x10000000)
            {
                // 0.15*$AP
                m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 15 / 100);
                return;
            }
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            // Rupture
            if (m_spellProto->SpellFamilyFlags[0] & 0x100000)
            {
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return;
                //1 point : ${($m1+$b1*1+0.015*$AP)*4} damage over 8 secs
                //2 points: ${($m1+$b1*2+0.024*$AP)*5} damage over 10 secs
                //3 points: ${($m1+$b1*3+0.03*$AP)*6} damage over 12 secs
                //4 points: ${($m1+$b1*4+0.03428571*$AP)*7} damage over 14 secs
                //5 points: ${($m1+$b1*5+0.0375*$AP)*8} damage over 16 secs
                float AP_per_combo[] = {0, 0.015f, 0.024, 0.03, 0.03428571, 0.0375};
                uint8 cp = ((Player*)caster)->GetComboPoints();
                if (cp > 5) cp = 5;
                m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * AP_per_combo[cp]);
                return;
            }
            // Garrote
            if (m_spellProto->SpellFamilyFlags[0] & 0x100)
            {
                // $AP*0.07 bonus per tick
                m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 7 / 100);
                return;
            }
            // Deadly Poison
            if (m_spellProto->SpellFamilyFlags[0] & 0x10000)
            {
                // 0.08*$AP / 4 * amount of stack
                m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 2 * GetParentAura()->GetStackAmount() / 100);
                return;
            }
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Serpent Sting
            if (m_spellProto->SpellFamilyFlags[0] & 0x4000)
            {
                // $RAP*0.1/5 bonus per tick
                m_amount += int32(caster->GetTotalAttackPowerValue(RANGED_ATTACK) * 10 / 500);
                return;
            }
            // Immolation Trap
            if (m_spellProto->SpellFamilyFlags[0] & 0x4 && m_spellProto->SpellIconID == 678)
            {
                // $RAP*0.1/5 bonus per tick
                m_amount += int32(caster->GetTotalAttackPowerValue(RANGED_ATTACK) * 10 / 500);
                return;
            }
            break;
        }
        default:
            break;
    }
}

void AuraEffect::HandlePeriodicDamagePCT(bool apply, bool Real)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandlePeriodicLeech(bool apply, bool Real)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandlePeriodicManaLeech(bool apply, bool Real)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandlePeriodicHealthFunnel(bool apply, bool Real)
{
    m_isPeriodic = apply;
}

/*********************************************************/
/***                  MODIFY STATS                     ***/
/*********************************************************/

/********************************/
/***        RESISTANCE        ***/
/********************************/

void AuraEffect::HandleAuraModResistanceExclusive(bool apply, bool Real)
{
    for(int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL;x++)
    {
        if(GetMiscValue() & int32(1<<x))
        {
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_VALUE, float(m_amount), apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                m_target->ApplyResistanceBuffModsMod(SpellSchools(x),GetParentAura()->IsPositive(),m_amount, apply);
        }
    }
}

void AuraEffect::HandleAuraModResistance(bool apply, bool Real)
{
    for(int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL;x++)
    {
        if(GetMiscValue() & int32(1<<x))
        {
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), TOTAL_VALUE, float(m_amount), apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER || ((Creature*)m_target)->isPet())
                m_target->ApplyResistanceBuffModsMod(SpellSchools(x),GetParentAura()->IsPositive(),m_amount, apply);
        }
    }

}

void AuraEffect::HandleAuraModBaseResistancePCT(bool apply, bool Real)
{
    // only players have base stats
    if(m_target->GetTypeId() != TYPEID_PLAYER)
    {
        //pets only have base armor
        if(((Creature*)m_target)->isPet() && (GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
            m_target->HandleStatModifier(UNIT_MOD_ARMOR, BASE_PCT, float(m_amount), apply);
    }
    else
    {
        for(int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL;x++)
        {
            if(GetMiscValue() & int32(1<<x))
                m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_PCT, float(m_amount), apply);
        }
    }
}

void AuraEffect::HandleModResistancePercent(bool apply, bool Real)
{
    for(int8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
    {
        if(GetMiscValue() & int32(1<<i))
        {
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_PCT, float(m_amount), apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER || ((Creature*)m_target)->isPet())
            {
                m_target->ApplyResistanceBuffModsPercentMod(SpellSchools(i),true,m_amount, apply);
                m_target->ApplyResistanceBuffModsPercentMod(SpellSchools(i),false,m_amount, apply);
            }
        }
    }
}

void AuraEffect::HandleModBaseResistance(bool apply, bool Real)
{
    // only players have base stats
    if(m_target->GetTypeId() != TYPEID_PLAYER)
    {
        //only pets have base stats
        if(((Creature*)m_target)->isPet() && (GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
            m_target->HandleStatModifier(UNIT_MOD_ARMOR, TOTAL_VALUE, float(m_amount), apply);
    }
    else
    {
        for(int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
            if(GetMiscValue() & (1<<i))
                m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_VALUE, float(m_amount), apply);
    }
}

/********************************/
/***           STAT           ***/
/********************************/

void AuraEffect::HandleAuraModStat(bool apply, bool Real)
{
    if (GetMiscValue() < -2 || GetMiscValue() > 4)
    {
        sLog.outError("WARNING: Spell %u effect %u have unsupported misc value (%i) for SPELL_AURA_MOD_STAT ",GetId(),GetEffIndex(),GetMiscValue());
        return;
    }

    for(int32 i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        // -1 or -2 is all stats ( misc < -2 checked in function beginning )
        if (GetMiscValue() < 0 || GetMiscValue() == i)
        {
            //m_target->ApplyStatMod(Stats(i), m_amount,apply);
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_VALUE, float(m_amount), apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER || ((Creature*)m_target)->isPet())
                m_target->ApplyStatBuffMod(Stats(i),m_amount,apply);
        }
    }
}

void AuraEffect::HandleModPercentStat(bool apply, bool Real)
{
    if (GetMiscValue() < -1 || GetMiscValue() > 4)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_PERCENT_STAT not valid");
        return;
    }

    // only players have base stats
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        if(GetMiscValue() == i || GetMiscValue() == -1)
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), BASE_PCT, float(m_amount), apply);
    }
}

void AuraEffect::HandleModSpellDamagePercentFromStat(bool /*apply*/, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Magic damage modifiers implemented in Unit::SpellDamageBonus
    // This information for client side use only
    // Recalculate bonus
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModSpellHealingPercentFromStat(bool /*apply*/, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate bonus
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleAuraModDispelResist(bool apply, bool Real)
{
    if(!Real || !apply)
        return;

    if(GetId()==33206)
        m_target->CastSpell(m_target,44416,true,NULL,this,GetCasterGUID());
}

void AuraEffect::HandleModSpellDamagePercentFromAttackPower(bool /*apply*/, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Magic damage modifiers implemented in Unit::SpellDamageBonus
    // This information for client side use only
    // Recalculate bonus
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModSpellHealingPercentFromAttackPower(bool /*apply*/, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate bonus
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModHealingDone(bool /*apply*/, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    // implemented in Unit::SpellHealingBonus
    // this information is for client side only
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModTotalPercentStat(bool apply, bool Real)
{
    if (GetMiscValue() < -1 || GetMiscValue() > 4)
    {
        sLog.outError("WARNING: Misc Value for SPELL_AURA_MOD_PERCENT_STAT not valid");
        return;
    }

    //save current and max HP before applying aura
    uint32 curHPValue = m_target->GetHealth();
    uint32 maxHPValue = m_target->GetMaxHealth();

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        if(GetMiscValue() == i || GetMiscValue() == -1)
        {
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT, float(m_amount), apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER || ((Creature*)m_target)->isPet())
                m_target->ApplyStatPercentBuffMod(Stats(i), m_amount, apply );
        }
    }

    //recalculate current HP/MP after applying aura modifications (only for spells with 0x10 flag)
    if ((GetMiscValue() == STAT_STAMINA) && (maxHPValue > 0) && (m_spellProto->Attributes & 0x10))
    {
        // newHP = (curHP / maxHP) * newMaxHP = (newMaxHP * curHP) / maxHP -> which is better because no int -> double -> int conversion is needed
        uint32 newHPValue = (m_target->GetMaxHealth() * curHPValue) / maxHPValue;
        m_target->SetHealth(newHPValue);
    }
}

void AuraEffect::HandleAuraModResistenceOfStatPercent(bool /*apply*/, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if(GetMiscValue() != SPELL_SCHOOL_MASK_NORMAL)
    {
        // support required adding replace UpdateArmor by loop by UpdateResistence at intellect update
        // and include in UpdateResistence same code as in UpdateArmor for aura mod apply.
        sLog.outError("Aura SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT(182) need adding support for non-armor resistances!");
        return;
    }

    // Recalculate Armor
    m_target->UpdateArmor();
}

/********************************/
/***      HEAL & ENERGIZE     ***/
/********************************/
void AuraEffect::HandleAuraModTotalHealthPercentRegen(bool apply, bool Real)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandleAuraModTotalEnergyPercentRegen(bool apply, bool Real)
{
    if(m_amplitude == 0)
        m_amplitude = 1000;

    m_periodicTimer = m_amplitude;
    m_isPeriodic = apply;
}

void AuraEffect::HandleModRegen(bool apply, bool Real)            // eating
{
    if(m_amplitude == 0)
        m_amplitude = 5000;

    m_periodicTimer = 5000;
    m_isPeriodic = apply;
}

void AuraEffect::HandleModPowerRegen(bool apply, bool Real)       // drinking
{
    if (!Real)
        return;

    Powers pt = m_target->getPowerType();
    if(m_amplitude == 0)
    {
        // Anger Management (only spell use this aura for rage)
        if (pt == POWER_RAGE)
            m_amplitude = 3000;
        else
            m_amplitude = 2000;
    }

    m_periodicTimer = 5000;

    if (m_target->GetTypeId() == TYPEID_PLAYER && GetMiscValue() == POWER_MANA)
        ((Player*)m_target)->UpdateManaRegen();

    m_isPeriodic = apply;
}

void AuraEffect::HandleModPowerRegenPCT(bool /*apply*/, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Update manaregen value
    if (GetMiscValue() == POWER_MANA)
        ((Player*)m_target)->UpdateManaRegen();
}

void AuraEffect::HandleModManaRegen(bool /*apply*/, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    //Note: an increase in regen does NOT cause threat.
    ((Player*)m_target)->UpdateManaRegen();
}

void AuraEffect::HandleComprehendLanguage(bool apply, bool Real)
{
    if(apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
}

void AuraEffect::HandleAuraModIncreaseHealth(bool apply, bool Real)
{
    if(Real)
    {
        if(apply)
        {
            m_target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_amount), apply);
            m_target->ModifyHealth(m_amount);
        }
        else
        {
            if (int32(m_target->GetHealth()) > m_amount)
                m_target->ModifyHealth(-m_amount);
            else
                m_target->SetHealth(1);
            m_target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_amount), apply);
        }
    }
}

void  AuraEffect::HandleAuraModIncreaseMaxHealth(bool apply, bool Real)
{
    uint32 oldhealth = m_target->GetHealth();
    double healthPercentage = (double)oldhealth / (double)m_target->GetMaxHealth();

    m_target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(m_amount), apply);

    // refresh percentage
    if(oldhealth > 0)
    {
        uint32 newhealth = uint32(ceil((double)m_target->GetMaxHealth() * healthPercentage));
        if(newhealth==0)
            newhealth = 1;

        m_target->SetHealth(newhealth);
    }
}

void AuraEffect::HandleAuraModIncreaseEnergy(bool apply, bool Real)
{
    Powers powerType = m_target->getPowerType();
    if(int32(powerType) != GetMiscValue())
        return;

    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + powerType);

    m_target->HandleStatModifier(unitMod, TOTAL_VALUE, float(m_amount), apply);
}

void AuraEffect::HandleAuraModIncreaseEnergyPercent(bool apply, bool /*Real*/)
{
    Powers powerType = m_target->getPowerType();
    if(int32(powerType) != GetMiscValue())
        return;

    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + powerType);

    m_target->HandleStatModifier(unitMod, TOTAL_PCT, float(m_amount), apply);
}

void AuraEffect::HandleAuraModIncreaseHealthPercent(bool apply, bool /*Real*/)
{
    m_target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_PCT, float(m_amount), apply);
}

void AuraEffect::HandleAuraIncreaseBaseHealthPercent(bool apply, bool /*Real*/)
{
    m_target->HandleStatModifier(UNIT_MOD_HEALTH, BASE_PCT, float(m_amount), apply);
}

/********************************/
/***          FIGHT           ***/
/********************************/

void AuraEffect::HandleAuraModParryPercent(bool /*apply*/, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    ((Player*)m_target)->UpdateParryPercentage();
}

void AuraEffect::HandleAuraModDodgePercent(bool /*apply*/, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    ((Player*)m_target)->UpdateDodgePercentage();
    //sLog.outError("BONUS DODGE CHANCE: + %f", float(m_amount));
}

void AuraEffect::HandleAuraModBlockPercent(bool /*apply*/, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    ((Player*)m_target)->UpdateBlockPercentage();
    //sLog.outError("BONUS BLOCK CHANCE: + %f", float(m_amount));
}

void AuraEffect::HandleAuraModRegenInterrupt(bool /*apply*/, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    ((Player*)m_target)->UpdateManaRegen();
}

void AuraEffect::HandleAuraModCritPercent(bool apply, bool Real)
{
    if(m_target->GetTypeId()!=TYPEID_PLAYER)
        return;

    // apply item specific bonuses for already equipped weapon
    if(Real)
    {
        for(int i = 0; i < MAX_ATTACK; ++i)
            if(Item* pItem = ((Player*)m_target)->GetWeaponForAttack(WeaponAttackType(i)))
                ((Player*)m_target)->_ApplyWeaponDependentAuraCritMod(pItem,WeaponAttackType(i),this,apply);
    }

    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // GetMiscValue() comparison with item generated damage types

    if (GetSpellProto()->EquippedItemClass == -1)
    {
        ((Player*)m_target)->HandleBaseModValue(CRIT_PERCENTAGE,         FLAT_MOD, float (m_amount), apply);
        ((Player*)m_target)->HandleBaseModValue(OFFHAND_CRIT_PERCENTAGE, FLAT_MOD, float (m_amount), apply);
        ((Player*)m_target)->HandleBaseModValue(RANGED_CRIT_PERCENTAGE,  FLAT_MOD, float (m_amount), apply);
    }
    else
    {
        // done in Player::_ApplyWeaponDependentAuraMods
    }
}

void AuraEffect::HandleModHitChance(bool apply, bool Real)
{
    if(m_target->GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)m_target)->UpdateMeleeHitChances();
        ((Player*)m_target)->UpdateRangedHitChances();
    }
    else
    {
        m_target->m_modMeleeHitChance += apply ? m_amount : (-m_amount);
        m_target->m_modRangedHitChance += apply ? m_amount : (-m_amount);
    }
}

void AuraEffect::HandleModSpellHitChance(bool apply, bool Real)
{
    if(m_target->GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)m_target)->UpdateSpellHitChances();
    }
    else
    {
        m_target->m_modSpellHitChance += apply ? m_amount: (-m_amount);
    }
}

void AuraEffect::HandleModSpellCritChance(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    if(m_target->GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)m_target)->UpdateAllSpellCritChances();
    }
    else
    {
        m_target->m_baseSpellCritChance += apply ? m_amount:-m_amount;
    }
}

void AuraEffect::HandleModSpellCritChanceShool(bool /*apply*/, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    for(int school = SPELL_SCHOOL_NORMAL; school < MAX_SPELL_SCHOOL; ++school)
        if (GetMiscValue() & (1<<school))
            ((Player*)m_target)->UpdateSpellCritChance(school);
}

/********************************/
/***         ATTACK SPEED     ***/
/********************************/

void AuraEffect::HandleModCastingSpeed(bool apply, bool Real)
{
    m_target->ApplyCastTimePercentMod(m_amount,apply);
}

void AuraEffect::HandleModMeleeRangedSpeedPct(bool apply, bool Real)
{
    m_target->ApplyAttackTimePercentMod(BASE_ATTACK,m_amount,apply);
    m_target->ApplyAttackTimePercentMod(OFF_ATTACK,m_amount,apply);
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK, m_amount, apply);
}

void AuraEffect::HandleModCombatSpeedPct(bool apply, bool Real)
{
    m_target->ApplyCastTimePercentMod(m_amount,apply);
    m_target->ApplyAttackTimePercentMod(BASE_ATTACK,m_amount,apply);
    m_target->ApplyAttackTimePercentMod(OFF_ATTACK,m_amount,apply);
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK, m_amount, apply);
}

void AuraEffect::HandleModAttackSpeed(bool apply, bool Real)
{
    if(!m_target->isAlive() )
        return;

    m_target->ApplyAttackTimePercentMod(BASE_ATTACK,m_amount,apply);
}

void AuraEffect::HandleHaste(bool apply, bool Real)
{
    m_target->ApplyAttackTimePercentMod(BASE_ATTACK,  m_amount,apply);
    m_target->ApplyAttackTimePercentMod(OFF_ATTACK,   m_amount,apply);
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK,m_amount,apply);
}

void AuraEffect::HandleAuraModRangedHaste(bool apply, bool Real)
{
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK, m_amount, apply);
}

void AuraEffect::HandleRangedAmmoHaste(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK,m_amount, apply);
}

/********************************/
/***        ATTACK POWER      ***/
/********************************/

void AuraEffect::HandleAuraModAttackPower(bool apply, bool Real)
{
    m_target->HandleStatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, float(m_amount), apply);
}

void AuraEffect::HandleAuraModRangedAttackPower(bool apply, bool Real)
{
    if((m_target->getClassMask() & CLASSMASK_WAND_USERS)!=0)
        return;

    m_target->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, float(m_amount), apply);
}

void AuraEffect::HandleAuraModAttackPowerPercent(bool apply, bool Real)
{
    //UNIT_FIELD_ATTACK_POWER_MULTIPLIER = multiplier - 1
    m_target->HandleStatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_PCT, float(m_amount), apply);
}

void AuraEffect::HandleAuraModRangedAttackPowerPercent(bool apply, bool Real)
{
    if((m_target->getClassMask() & CLASSMASK_WAND_USERS)!=0)
        return;

    //UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER = multiplier - 1
    m_target->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_PCT, float(m_amount), apply);
}

void AuraEffect::HandleAuraModRangedAttackPowerOfStatPercent(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    // Recalculate bonus
    if(m_target->GetTypeId() == TYPEID_PLAYER && !(m_target->getClassMask() & CLASSMASK_WAND_USERS))
        ((Player*)m_target)->UpdateAttackPowerAndDamage(true);
}

void AuraEffect::HandleAuraModAttackPowerOfStatPercent(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    // Recalculate bonus
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_target)->UpdateAttackPowerAndDamage(false);
}

/********************************/
/***        DAMAGE BONUS      ***/
/********************************/
void AuraEffect::HandleModDamageDone(bool apply, bool Real)
{
    // apply item specific bonuses for already equipped weapon
    if(Real && m_target->GetTypeId()==TYPEID_PLAYER)
    {
        for(int i = 0; i < MAX_ATTACK; ++i)
            if(Item* pItem = ((Player*)m_target)->GetWeaponForAttack(WeaponAttackType(i)))
                ((Player*)m_target)->_ApplyWeaponDependentAuraDamageMod(pItem,WeaponAttackType(i),this,apply);
    }

    // GetMiscValue() is bitmask of spell schools
    // 1 ( 0-bit ) - normal school damage (SPELL_SCHOOL_MASK_NORMAL)
    // 126 - full bitmask all magic damages (SPELL_SCHOOL_MASK_MAGIC) including wands
    // 127 - full bitmask any damages
    //
    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // GetMiscValue() comparison with item generated damage types

    if((GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL) != 0)
    {
        // apply generic physical damage bonuses including wand case
        if (GetSpellProto()->EquippedItemClass == -1 || m_target->GetTypeId() != TYPEID_PLAYER)
        {
            m_target->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_VALUE, float(m_amount), apply);
            m_target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_VALUE, float(m_amount), apply);
            m_target->HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_VALUE, float(m_amount), apply);
        }
        else
        {
            // done in Player::_ApplyWeaponDependentAuraMods
        }

        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            if(GetParentAura()->IsPositive())
                m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS,m_amount,apply);
            else
                m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG,m_amount,apply);
        }
    }

    // Skip non magic case for speedup
    if((GetMiscValue() & SPELL_SCHOOL_MASK_MAGIC) == 0)
        return;

    if( GetSpellProto()->EquippedItemClass != -1 || GetSpellProto()->EquippedItemInventoryTypeMask != 0 )
    {
        // wand magic case (skip generic to all item spell bonuses)
        // done in Player::_ApplyWeaponDependentAuraMods

        // Skip item specific requirements for not wand magic damage
        return;
    }

    // Magic damage modifiers implemented in Unit::SpellDamageBonus
    // This information for client side use only
    if(m_target->GetTypeId() == TYPEID_PLAYER)
    {
        if(GetParentAura()->IsPositive())
        {
            for(int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; i++)
            {
                if((GetMiscValue() & (1<<i)) != 0)
                    m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS+i,m_amount,apply);
            }
        }
        else
        {
            for(int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; i++)
            {
                if((GetMiscValue() & (1<<i)) != 0)
                    m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG+i,m_amount,apply);
            }
        }
        Pet* pet = ((Player*)m_target)->GetPet();
        if(pet)
            pet->UpdateAttackPowerAndDamage();
    }
}

void AuraEffect::HandleModDamagePercentDone(bool apply, bool Real)
{
    sLog.outDebug("AURA MOD DAMAGE type:%u negative:%u", GetMiscValue(), GetParentAura()->IsPositive() ? 0 : 1);

    // apply item specific bonuses for already equipped weapon
    if(Real && m_target->GetTypeId()==TYPEID_PLAYER)
    {
        for(int i = 0; i < MAX_ATTACK; ++i)
            if(Item* pItem = ((Player*)m_target)->GetWeaponForAttack(WeaponAttackType(i)))
                ((Player*)m_target)->_ApplyWeaponDependentAuraDamageMod(pItem,WeaponAttackType(i),this,apply);
    }

    // GetMiscValue() is bitmask of spell schools
    // 1 ( 0-bit ) - normal school damage (SPELL_SCHOOL_MASK_NORMAL)
    // 126 - full bitmask all magic damages (SPELL_SCHOOL_MASK_MAGIC) including wand
    // 127 - full bitmask any damages
    //
    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // GetMiscValue() comparison with item generated damage types

    if((GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL) != 0)
    {
        // apply generic physical damage bonuses including wand case
        if (GetSpellProto()->EquippedItemClass == -1 || m_target->GetTypeId() != TYPEID_PLAYER)
        {
            m_target->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, float(m_amount), apply);
            m_target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT, float(m_amount), apply);
            m_target->HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_PCT, float(m_amount), apply);
        }
        else
        {
            // done in Player::_ApplyWeaponDependentAuraMods
        }
        // For show in client
        if(m_target->GetTypeId() == TYPEID_PLAYER)
            m_target->ApplyModSignedFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT,m_amount/100.0f,apply);
    }

    // Skip non magic case for speedup
    if((GetMiscValue() & SPELL_SCHOOL_MASK_MAGIC) == 0)
        return;

    if( GetSpellProto()->EquippedItemClass != -1 || GetSpellProto()->EquippedItemInventoryTypeMask != 0 )
    {
        // wand magic case (skip generic to all item spell bonuses)
        // done in Player::_ApplyWeaponDependentAuraMods

        // Skip item specific requirements for not wand magic damage
        return;
    }

    // Magic damage percent modifiers implemented in Unit::SpellDamageBonus
    // Send info to client
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        for(int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            m_target->ApplyModSignedFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT+i,m_amount/100.0f,apply);
}

void AuraEffect::HandleModOffhandDamagePercent(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    sLog.outDebug("AURA MOD OFFHAND DAMAGE");

    m_target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT, float(m_amount), apply);
}

/********************************/
/***        POWER COST        ***/
/********************************/

void AuraEffect::HandleModPowerCostPCT(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    float amount = m_amount /100.0f;
    for(int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if(GetMiscValue() & (1<<i))
            m_target->ApplyModSignedFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER+i,amount,apply);
}

void AuraEffect::HandleModPowerCost(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    for(int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if(GetMiscValue() & (1<<i))
            m_target->ApplyModInt32Value(UNIT_FIELD_POWER_COST_MODIFIER+i,m_amount,apply);
}

void AuraEffect::HandleNoReagentUseAura(bool Apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    flag96 mask;
    Unit::AuraEffectList const& noReagent = m_target->GetAurasByType(SPELL_AURA_NO_REAGENT_USE);
        for(Unit::AuraEffectList::const_iterator i = noReagent.begin(); i !=  noReagent.end(); ++i)
            mask |= (*i)->m_spellProto->EffectSpellClassMask[(*i)->m_effIndex];

    m_target->SetUInt32Value(PLAYER_NO_REAGENT_COST_1  , mask[0]);
    m_target->SetUInt32Value(PLAYER_NO_REAGENT_COST_1+1, mask[1]);
    m_target->SetUInt32Value(PLAYER_NO_REAGENT_COST_1+2, mask[2]);
}

/*********************************************************/
/***                    OTHERS                         ***/
/*********************************************************/

void AuraEffect::HandleAuraAllowOnlyAbility(bool apply, bool Real)
{
    if(!Real)
        return;

    if(!apply && m_target->HasAuraType(SPELL_AURA_ALLOW_ONLY_ABILITY))
        return;

    if(m_target->GetTypeId()==TYPEID_PLAYER)
    {
        if (apply)
            m_target->SetFlag(PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY);
        else
            m_target->RemoveFlag(PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY);
    }
}

void AuraEffect::HandleAuraEmpathy(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_UNIT)
        return;

    CreatureInfo const * ci = objmgr.GetCreatureTemplate(m_target->GetEntry());
    if(ci && ci->type == CREATURE_TYPE_BEAST)
        m_target->ApplyModUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_SPECIALINFO, apply);
}

void AuraEffect::HandleAuraUntrackable(bool apply, bool Real)
{
    if(apply)
        m_target->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_UNTRACKABLE);
    else
        m_target->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_UNTRACKABLE);
}

void AuraEffect::HandleAuraModPacify(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if(apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
}

void AuraEffect::HandleAuraModPacifyAndSilence(bool apply, bool Real)
{
    HandleAuraModPacify(apply,Real);
    HandleAuraModSilence(apply,Real);
}

void AuraEffect::HandleAuraGhost(bool apply, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if(apply)
    {
        m_target->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
    }
    else
    {
        m_target->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
    }
}

void AuraEffect::HandleAuraAllowFlight(bool apply, bool Real)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    // allow fly
    WorldPacket data;
    if(apply)
    {
        data.Initialize(SMSG_MOVE_SET_CAN_FLY, 12);
        if(m_target->GetTypeId() == TYPEID_UNIT)
        {
            m_target->SetByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
            m_target->AddUnitMovementFlag(MOVEMENTFLAG_FLYING2);
        }
    }
    else
    {
        data.Initialize(SMSG_MOVE_UNSET_CAN_FLY, 12);
        if(m_target->GetTypeId() == TYPEID_UNIT)
        {
            m_target->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
            m_target->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING2);
        }
    }
    data.append(m_target->GetPackGUID());
    data << uint32(0);                                      // unk
    m_target->SendMessageToSet(&data, true);
}

void AuraEffect::HandleModRating(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
        if (GetMiscValue() & (1 << rating))
            ((Player*)m_target)->ApplyRatingMod(CombatRating(rating), m_amount, apply);
}

void AuraEffect::HandleModRatingFromStat(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    // Just recalculate ratings
    for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
        if (GetMiscValue() & (1 << rating))
            ((Player*)m_target)->ApplyRatingMod(CombatRating(rating), 0, apply);
}

void AuraEffect::HandleForceMoveForward(bool apply, bool Real)
{
    if(!Real || m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    if(apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVE);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVE);
}

void AuraEffect::HandleAuraModExpertise(bool /*apply*/, bool Real)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)m_target)->UpdateExpertise(BASE_ATTACK);
    ((Player*)m_target)->UpdateExpertise(OFF_ATTACK);
}

void AuraEffect::HandleModTargetResistance(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;
    // applied to damage as HandleNoImmediateEffect in Unit::CalcAbsorbResist and Unit::CalcArmorReducedDamage

    // show armor penetration
    if (m_target->GetTypeId() == TYPEID_PLAYER && (GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
        m_target->ApplyModInt32Value(PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE,m_amount, apply);

    // show as spell penetration only full spell penetration bonuses (all resistances except armor and holy
    if (m_target->GetTypeId() == TYPEID_PLAYER && (GetMiscValue() & SPELL_SCHOOL_MASK_SPELL)==SPELL_SCHOOL_MASK_SPELL)
        m_target->ApplyModInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE,m_amount, apply);
}

void AuraEffect::HandleShieldBlockValue(bool apply, bool Real)
{
    BaseModType modType = FLAT_MOD;
    if(m_auraName == SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT)
        modType = PCT_MOD;

    if(m_target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_target)->HandleBaseModValue(SHIELD_BLOCK_VALUE, modType, float(m_amount), apply);
}

void AuraEffect::HandleAuraRetainComboPoints(bool apply, bool Real)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *target = (Player*)m_target;

    // combo points was added in SPELL_EFFECT_ADD_COMBO_POINTS handler
    // remove only if aura expire by time (in case combo points amount change aura removed without combo points lost)
    if( !apply && GetParentAura()->GetAuraDuration()==0 && target->GetComboTarget())
        if(Unit* unit = ObjectAccessor::GetUnit(*m_target,target->GetComboTarget()))
            target->AddComboPoints(unit, -m_amount);
}

void AuraEffect::HandleModUnattackable( bool Apply, bool Real )
{
    if(!Real)
        return;

    if(Apply)
    {
        m_target->CombatStop();
        m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);
    }

    m_target->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE, Apply);
}

void AuraEffect::HandleSpiritOfRedemption( bool apply, bool Real )
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    // prepare spirit state
    if(apply)
    {
        if(m_target->GetTypeId()==TYPEID_PLAYER)
        {
            // disable breath/etc timers
            ((Player*)m_target)->StopMirrorTimers();

            // set stand state (expected in this form)
            if(!m_target->IsStandState())
                m_target->SetStandState(UNIT_STAND_STATE_STAND);
        }

        m_target->SetHealth(1);
    }
    // die at aura end
    else
        m_target->setDeathState(JUST_DIED);
}

void AuraEffect::HandleSchoolAbsorb(bool apply, bool Real)
{
    if(!Real)
        return;

    // prevent double apply bonuses
    if(apply && (m_target->GetTypeId()!=TYPEID_PLAYER || !((Player*)m_target)->GetSession()->PlayerLoading()))
    {
        if(Unit* caster = GetCaster())
        {
            float DoneActualBenefit = 0.0f;
            switch(m_spellProto->SpellFamilyName)
            {
                case SPELLFAMILY_PRIEST:
                    if(m_spellProto->SpellFamilyFlags.IsEqual(0x1)) //PW:S
                    {
                        //+30% from +healing bonus
                        DoneActualBenefit = caster->SpellBaseHealingBonus(GetSpellSchoolMask(m_spellProto)) * 0.3f;
                    }
                    break;
                case SPELLFAMILY_MAGE:
                    if(m_spellProto->SpellFamilyFlags.IsEqual(0x80100) || m_spellProto->SpellFamilyFlags.IsEqual(0x8) || m_spellProto->SpellFamilyFlags.IsEqual(0,0x1))
                    {
                        //frost ward, fire ward, ice barrier
                        //+10% from +spd bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonus(GetSpellSchoolMask(m_spellProto)) * 0.1f;
                    }
                    break;
                case SPELLFAMILY_WARLOCK:
                    if(m_spellProto->SpellFamilyFlags.IsEqual(0,0,0x40))
                    {
                        //shadow ward
                        //+10% from +spd bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonus(GetSpellSchoolMask(m_spellProto)) * 0.1f;
                    }
                    break;
                default:
                    break;
            }

            DoneActualBenefit *= caster->CalculateLevelPenalty(GetSpellProto());

            m_amount += (int32)DoneActualBenefit;
        }
    }
}

void AuraEffect::PeriodicTick()
{
    if(!m_target->isAlive())
        return;

    switch(GetAuraName())
    {
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
        {
            Unit *pCaster = GetCaster();
            if(!pCaster)
                return;

            if( GetSpellProto()->Effect[GetEffIndex()]==SPELL_EFFECT_PERSISTENT_AREA_AURA &&
                pCaster->SpellHitResult(m_target,GetSpellProto(),false)!=SPELL_MISS_NONE)
                return;

            // Check for immune (not use charges)
            if(m_target->IsImmunedToDamage(GetSpellSchoolMask(GetSpellProto())))
                return;

            // some auras remove at specific health level or more
            if(m_auraName==SPELL_AURA_PERIODIC_DAMAGE)
            {
                switch(GetId())
                {
                    case 43093: case 31956: case 38801:
                    case 35321: case 38363: case 39215:
                        if(m_target->GetHealth() == m_target->GetMaxHealth() )
                        {
                            m_target->RemoveAurasDueToSpell(GetId());
                            return;
                        }
                        break;
                    case 38772:
                    {
                        uint32 percent =
                            GetEffIndex() < 2 && GetSpellProto()->Effect[GetEffIndex()]==SPELL_EFFECT_DUMMY ?
                            pCaster->CalculateSpellDamage(GetSpellProto(),GetEffIndex()+1,GetSpellProto()->EffectBasePoints[GetEffIndex()+1],m_target) :
                            100;
                        if(m_target->GetHealth()*100 >= m_target->GetMaxHealth()*percent )
                        {
                            m_target->RemoveAurasDueToSpell(GetId());
                            return;
                        }
                        break;
                    }
                    case 41337:// aura of anger
                    {
                        Unit::AuraEffectList const& mMod = m_target->GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
                        for(Unit::AuraEffectList::const_iterator i = mMod.begin(); i != mMod.end(); ++i)
                        {
                            if ((*i)->GetId() == 41337)
                            {
                                (*i)->ApplyModifier(false);
                                (*i)->SetAmount((*i)->GetAmount()+5);
                                (*i)->ApplyModifier(true);
                                break;
                            }
                        }
                        m_amount += 100;
                    }break;
                    default:
                        break;
                }
            }

            uint32 absorb=0;
            uint32 resist=0;
            CleanDamage cleanDamage =  CleanDamage(0, BASE_ATTACK, MELEE_HIT_NORMAL );

            // ignore non positive values (can be result apply spellmods to aura damage
            //uint32 amount = GetModifierValuePerStack() > 0 ? GetModifierValuePerStack() : 0;
            uint32 pdamage = GetAmount() > 0 ? GetAmount() : 0;

            if(GetAuraName() == SPELL_AURA_PERIODIC_DAMAGE)
            {
                pdamage = pCaster->SpellDamageBonus(m_target, GetSpellProto(), pdamage, DOT, GetParentAura()->GetStackAmount());

                // Calculate armor mitigation if it is a physical spell
                // But not for bleed mechanic spells
                if ( GetSpellSchoolMask(GetSpellProto()) & SPELL_SCHOOL_MASK_NORMAL &&
                     GetEffectMechanic(GetSpellProto(), m_effIndex) != MECHANIC_BLEED)
                {
                    uint32 pdamageReductedArmor = pCaster->CalcArmorReducedDamage(m_target, pdamage, GetSpellProto());
                    cleanDamage.damage += pdamage - pdamageReductedArmor;
                    pdamage = pdamageReductedArmor;
                }

                // Curse of Agony damage-per-tick calculation
                if (GetSpellProto()->SpellFamilyName==SPELLFAMILY_WARLOCK && (GetSpellProto()->SpellFamilyFlags[0] & 0x400) && GetSpellProto()->SpellIconID==544)
                {
                    // 1..4 ticks, 1/2 from normal tick damage
                    if (GetParentAura()->GetAuraDuration()>=((GetParentAura()->GetAuraMaxDuration()-m_amplitude)*2/3))
                        pdamage = pdamage/2;
                    // 9..12 ticks, 3/2 from normal tick damage
                    else if(GetParentAura()->GetAuraDuration()<((GetParentAura()->GetAuraMaxDuration()-m_amplitude)/3))
                        pdamage += (pdamage+1)/2;           // +1 prevent 0.5 damage possible lost at 1..4 ticks
                    // 5..8 ticks have normal tick damage
                }
            }
            else
                pdamage = uint32(m_target->GetMaxHealth()*pdamage/100);

            //As of 2.2 resilience reduces damage from DoT ticks as much as the chance to not be critically hit
            // Reduce dot damage from resilience for players
            if (m_target->GetTypeId()==TYPEID_PLAYER)
                pdamage-=((Player*)m_target)->GetDotDamageReduction(pdamage);

            pCaster->CalcAbsorbResist(m_target, GetSpellSchoolMask(GetSpellProto()), DOT, pdamage, &absorb, &resist, m_spellProto);

            sLog.outDetail("PeriodicTick: %u (TypeId: %u) attacked %u (TypeId: %u) for %u dmg inflicted by %u abs is %u",
                GUID_LOPART(GetCasterGUID()), GuidHigh2TypeId(GUID_HIPART(GetCasterGUID())), m_target->GetGUIDLow(), m_target->GetTypeId(), pdamage, GetId(),absorb);

            WorldPacket data(SMSG_PERIODICAURALOG, (21+16));// we guess size
            data.append(m_target->GetPackGUID());
            data.appendPackGUID(GetCasterGUID());
            data << uint32(GetId());
            data << uint32(1);
            data << uint32(m_auraName);
            data << (uint32)pdamage;
            data << uint32(0);                              // overkill
            data << (uint32)GetSpellSchoolMask(GetSpellProto()); // will be mask in 2.4.x
            data << (uint32)absorb;
            data << (uint32)resist;
            m_target->SendMessageToSet(&data,true);

            Unit* target = m_target;                        // aura can be deleted in DealDamage
            SpellEntry const* spellProto = GetSpellProto();

            // Set trigger flag
            uint32 procAttacker = PROC_FLAG_ON_DO_PERIODIC;//   | PROC_FLAG_SUCCESSFUL_HARMFUL_SPELL_HIT;
            uint32 procVictim   = PROC_FLAG_ON_TAKE_PERIODIC;// | PROC_FLAG_TAKEN_HARMFUL_SPELL_HIT;
            pdamage = (pdamage <= absorb+resist) ? 0 : (pdamage-absorb-resist);
            if (pdamage)
                procVictim|=PROC_FLAG_TAKEN_ANY_DAMAGE;
            pCaster->ProcDamageAndSpell(target, procAttacker, procVictim, PROC_EX_NORMAL_HIT, pdamage, BASE_ATTACK, spellProto);

            pCaster->DealDamage(target, pdamage, &cleanDamage, DOT, GetSpellSchoolMask(spellProto), spellProto, true);
            break;
        }
        case SPELL_AURA_PERIODIC_LEECH:
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
        {
            Unit *pCaster = GetCaster();
            if(!pCaster)
                return;

            if(!pCaster->isAlive())
                return;

            if( GetSpellProto()->Effect[GetEffIndex()]==SPELL_EFFECT_PERSISTENT_AREA_AURA &&
                pCaster->SpellHitResult(m_target,GetSpellProto(),false)!=SPELL_MISS_NONE)
                return;

            // Check for immune
            if(m_target->IsImmunedToDamage(GetSpellSchoolMask(GetSpellProto())))
                return;

            uint32 absorb=0;
            uint32 resist=0;
            CleanDamage cleanDamage =  CleanDamage(0, BASE_ATTACK, MELEE_HIT_NORMAL );

            //uint32 pdamage = GetModifierValuePerStack() > 0 ? GetModifierValuePerStack() : 0;
            uint32 pdamage = GetAmount() > 0 ? GetAmount() : 0;
            pdamage = pCaster->SpellDamageBonus(m_target, GetSpellProto(), pdamage, DOT, GetParentAura()->GetStackAmount());

            //Calculate armor mitigation if it is a physical spell
            if (GetSpellSchoolMask(GetSpellProto()) & SPELL_SCHOOL_MASK_NORMAL)
            {
                uint32 pdamageReductedArmor = pCaster->CalcArmorReducedDamage(m_target, pdamage, GetSpellProto());
                cleanDamage.damage += pdamage - pdamageReductedArmor;
                pdamage = pdamageReductedArmor;
            }

            //As of 2.2 resilience reduces damage from DoT ticks as much as the chance to not be critically hit
            // Reduce dot damage from resilience for players
            if (m_target->GetTypeId()==TYPEID_PLAYER)
                pdamage-=((Player*)m_target)->GetDotDamageReduction(pdamage);

            pCaster->CalcAbsorbResist(m_target, GetSpellSchoolMask(GetSpellProto()), DOT, pdamage, &absorb, &resist, m_spellProto);

            if(m_target->GetHealth() < pdamage)
                pdamage = uint32(m_target->GetHealth());

            sLog.outDetail("PeriodicTick: %u (TypeId: %u) health leech of %u (TypeId: %u) for %u dmg inflicted by %u abs is %u",
                GUID_LOPART(GetCasterGUID()), GuidHigh2TypeId(GUID_HIPART(GetCasterGUID())), m_target->GetGUIDLow(), m_target->GetTypeId(), pdamage, GetId(),absorb);

            pCaster->SendSpellNonMeleeDamageLog(m_target, GetId(), pdamage, GetSpellSchoolMask(GetSpellProto()), absorb, resist, false, 0);

            Unit* target = m_target;                        // aura can be deleted in DealDamage
            SpellEntry const* spellProto = GetSpellProto();
            float multiplier = spellProto->EffectMultipleValue[GetEffIndex()] > 0 ? spellProto->EffectMultipleValue[GetEffIndex()] : 1;
            int32 stackAmount = GetParentAura()->GetStackAmount();

            // Set trigger flag
            uint32 procAttacker = PROC_FLAG_ON_DO_PERIODIC;//   | PROC_FLAG_SUCCESSFUL_HARMFUL_SPELL_HIT;
            uint32 procVictim   = PROC_FLAG_ON_TAKE_PERIODIC;// | PROC_FLAG_TAKEN_HARMFUL_SPELL_HIT;
            pdamage = (pdamage <= absorb+resist) ? 0 : (pdamage-absorb-resist);
            if (pdamage)
                procVictim|=PROC_FLAG_TAKEN_ANY_DAMAGE;
            pCaster->ProcDamageAndSpell(target, procAttacker, procVictim, PROC_EX_NORMAL_HIT, pdamage, BASE_ATTACK, spellProto);
            int32 new_damage = pCaster->DealDamage(target, pdamage, &cleanDamage, DOT, GetSpellSchoolMask(spellProto), spellProto, false);

            if (!target->isAlive() && pCaster->IsNonMeleeSpellCasted(false))
            {
                for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
                {
                    if (pCaster->m_currentSpells[i] && pCaster->m_currentSpells[i]->m_spellInfo->Id == spellProto->Id)
                        pCaster->m_currentSpells[i]->cancel();
                }
            }


            if(Player *modOwner = pCaster->GetSpellModOwner())
                modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_MULTIPLE_VALUE, multiplier);

            // Don't apply heal mods for this aura
            uint32 heal = uint32(new_damage * multiplier);//pCaster->SpellHealingBonus(pCaster, spellProto, uint32(new_damage * multiplier), DOT, stackAmount);

            int32 gain = pCaster->ModifyHealth(heal);
            pCaster->getHostilRefManager().threatAssist(pCaster, gain * 0.5f, spellProto);

            pCaster->SendHealSpellLog(pCaster, spellProto->Id, heal);
            break;
        }
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_OBS_MOD_HEALTH:
        {
            Unit *pCaster = GetCaster();
            if(!pCaster)
                return;

            // heal for caster damage (must be alive)
            if(m_target != pCaster && GetSpellProto()->SpellVisual[0]==163 && !pCaster->isAlive())
                return;

            if(GetParentAura()->GetAuraDuration() ==-1 && m_target->GetHealth()==m_target->GetMaxHealth())
                return;

            // ignore non positive values (can be result apply spellmods to aura damage
            //uint32 amount = GetModifierValuePerStack() > 0 ? GetModifierValuePerStack() : 0;
            int32 pdamage = GetAmount() > 0 ? GetAmount() : 0;

            if(m_auraName==SPELL_AURA_OBS_MOD_HEALTH)
                pdamage = uint32(m_target->GetMaxHealth() * pdamage * GetParentAura()->GetStackAmount() / 100);
            else
                pdamage = pCaster->SpellHealingBonus(m_target, GetSpellProto(), pdamage, DOT, GetParentAura()->GetStackAmount());

            sLog.outDetail("PeriodicTick: %u (TypeId: %u) heal of %u (TypeId: %u) for %u health inflicted by %u",
                GUID_LOPART(GetCasterGUID()), GuidHigh2TypeId(GUID_HIPART(GetCasterGUID())), m_target->GetGUIDLow(), m_target->GetTypeId(), pdamage, GetId());

            WorldPacket data(SMSG_PERIODICAURALOG, (21+16));// we guess size
            data.append(m_target->GetPackGUID());
            data.appendPackGUID(GetCasterGUID());
            data << uint32(GetId());
            data << uint32(1);
            data << uint32(m_auraName);
            data << (uint32)pdamage;
            data << uint32(0);                              // wotlk
            m_target->SendMessageToSet(&data,true);

            int32 gain = m_target->ModifyHealth(pdamage);

            // add HoTs to amount healed in bgs
            if( pCaster->GetTypeId() == TYPEID_PLAYER )
                if( BattleGround *bg = ((Player*)pCaster)->GetBattleGround() )
                    bg->UpdatePlayerScore(((Player*)pCaster), SCORE_HEALING_DONE, gain);

            //Do check before because m_modifier.auraName can be invalidate by DealDamage.
            bool procSpell = (m_auraName == SPELL_AURA_PERIODIC_HEAL && m_target != pCaster);

            m_target->getHostilRefManager().threatAssist(pCaster, float(gain) * 0.5f, GetSpellProto());

            Unit* target = m_target;                        // aura can be deleted in DealDamage
            SpellEntry const* spellProto = GetSpellProto();
            bool haveCastItem = GetParentAura()->GetCastItemGUID()!=0;

            // heal for caster damage
            if(m_target!=pCaster && spellProto->SpellVisual[0]==163)
            {
                uint32 dmg = spellProto->manaPerSecond;
                if(pCaster->GetHealth() <= dmg && pCaster->GetTypeId()==TYPEID_PLAYER)
                {
                    pCaster->RemoveAurasDueToSpell(GetId());

                    // finish current generic/channeling spells, don't affect autorepeat
                    if(pCaster->m_currentSpells[CURRENT_GENERIC_SPELL])
                    {
                        pCaster->m_currentSpells[CURRENT_GENERIC_SPELL]->finish();
                    }
                    if(pCaster->m_currentSpells[CURRENT_CHANNELED_SPELL])
                    {
                        pCaster->m_currentSpells[CURRENT_CHANNELED_SPELL]->SendChannelUpdate(0);
                        pCaster->m_currentSpells[CURRENT_CHANNELED_SPELL]->finish();
                    }
                }
                else
                {
                    pCaster->SendSpellNonMeleeDamageLog(pCaster, GetId(), gain, GetSpellSchoolMask(GetSpellProto()), 0, 0, false, 0, false);

                    CleanDamage cleanDamage =  CleanDamage(0, BASE_ATTACK, MELEE_HIT_NORMAL );
                    pCaster->DealDamage(pCaster, gain, &cleanDamage, NODAMAGE, GetSpellSchoolMask(GetSpellProto()), GetSpellProto(), true);
                }
            }

            uint32 procAttacker = PROC_FLAG_ON_DO_PERIODIC;//   | PROC_FLAG_SUCCESSFUL_HEAL;
            uint32 procVictim   = 0;//ROC_FLAG_ON_TAKE_PERIODIC | PROC_FLAG_TAKEN_HEAL;
            // ignore item heals
//            if(procSpell && !haveCastItem)
//                pCaster->ProcDamageAndSpell(target, procAttacker, procVictim, PROC_EX_NORMAL_HIT, pdamage, BASE_ATTACK, spellProto);
            break;
        }
        case SPELL_AURA_PERIODIC_MANA_LEECH:
        {
            if(GetMiscValue() < 0 || GetMiscValue() >= MAX_POWERS)
                return;

            Powers power = Powers(GetMiscValue());

            // power type might have changed between aura applying and tick (druid's shapeshift)
            if(m_target->getPowerType() != power)
                return;

            Unit *pCaster = GetCaster();
            if(!pCaster)
                return;

            if(!pCaster->isAlive())
                return;

            if( GetSpellProto()->Effect[GetEffIndex()]==SPELL_EFFECT_PERSISTENT_AREA_AURA &&
                pCaster->SpellHitResult(m_target,GetSpellProto(),false)!=SPELL_MISS_NONE)
                return;

            // Check for immune (not use charges)
            if(m_target->IsImmunedToDamage(GetSpellSchoolMask(GetSpellProto())))
                return;

            // ignore non positive values (can be result apply spellmods to aura damage
            uint32 pdamage = m_amount > 0 ? m_amount : 0;

            // Special case: draining x% of mana (up to a maximum of 2*x% of the caster's maximum mana)
            // It's mana percent cost spells, m_amount is percent drain from target
            if (m_spellProto->ManaCostPercentage)
            {
                // max value
                uint32 maxmana = pCaster->GetMaxPower(power)  * pdamage * 2 / 100;
                pdamage = m_target->GetMaxPower(power) * pdamage / 100;
                if(pdamage > maxmana)
                    pdamage = maxmana;
            }

            sLog.outDetail("PeriodicTick: %u (TypeId: %u) power leech of %u (TypeId: %u) for %u dmg inflicted by %u",
                GUID_LOPART(GetCasterGUID()), GuidHigh2TypeId(GUID_HIPART(GetCasterGUID())), m_target->GetGUIDLow(), m_target->GetTypeId(), pdamage, GetId());

            int32 drain_amount = m_target->GetPower(power) > pdamage ? pdamage : m_target->GetPower(power);

            // resilience reduce mana draining effect at spell crit damage reduction (added in 2.4)
            if (power == POWER_MANA && m_target->GetTypeId() == TYPEID_PLAYER)
                drain_amount -= ((Player*)m_target)->GetSpellCritDamageReduction(drain_amount);

            m_target->ModifyPower(power, -drain_amount);

            float gain_multiplier = 0;

            if(pCaster->GetMaxPower(power) > 0)
            {
                gain_multiplier = GetSpellProto()->EffectMultipleValue[GetEffIndex()];

                if(Player *modOwner = pCaster->GetSpellModOwner())
                    modOwner->ApplySpellMod(GetId(), SPELLMOD_MULTIPLE_VALUE, gain_multiplier);
            }

            WorldPacket data(SMSG_PERIODICAURALOG, (21+16));// we guess size
            data.append(m_target->GetPackGUID());
            data.appendPackGUID(GetCasterGUID());
            data << uint32(GetId());
            data << uint32(1);
            data << uint32(m_auraName);
            data << (uint32)power;                          // power type
            data << (uint32)drain_amount;
            data << (float)gain_multiplier;
            m_target->SendMessageToSet(&data,true);

            int32 gain_amount = int32(drain_amount*gain_multiplier);

            if(gain_amount)
            {
                int32 gain = pCaster->ModifyPower(power,gain_amount);
                m_target->AddThreat(pCaster, float(gain) * 0.5f, GetSpellSchoolMask(GetSpellProto()), GetSpellProto());
            }

            // Mark of Kaz'rogal
            if(GetId() == 31447 && m_target->GetPower(power) == 0)
            {
                m_target->CastSpell(m_target, 31463, true, 0, this);
                // Remove aura
                GetParentAura()->SetAuraDuration(0);
            }

            // Mark of Kazzak
            if(GetId() == 32960)
            {
                int32 modifier = (m_target->GetPower(power) * 0.05f);
                m_target->ModifyPower(power, -modifier);

                if(m_target->GetPower(power) == 0)
                {
                    m_target->CastSpell(m_target, 32961, true, 0, this);
                    // Remove aura
                    GetParentAura()->SetAuraDuration(0);
                }
            }
            break;
        }
        case SPELL_AURA_OBS_MOD_ENERGY:
        {
            if(GetMiscValue() < 0)
                return;

            Powers power;
            if (GetMiscValue() == POWER_ALL)
                power = m_target->getPowerType();
            else
                power = Powers(GetMiscValue());

            if(m_target->GetMaxPower(power) == 0)
                return;

            if(GetParentAura()->GetAuraDuration() ==-1 && m_target->GetPower(power)==m_target->GetMaxPower(power))
                return;

            uint32 amount = m_amount * m_target->GetMaxPower(power) /100;
            sLog.outDetail("PeriodicTick: %u (TypeId: %u) energize %u (TypeId: %u) for %u dmg inflicted by %u",
                GUID_LOPART(GetCasterGUID()), GuidHigh2TypeId(GUID_HIPART(GetCasterGUID())), m_target->GetGUIDLow(), m_target->GetTypeId(), amount, GetId());

            WorldPacket data(SMSG_PERIODICAURALOG, (21+16));// we guess size
            data.append(m_target->GetPackGUID());
            data.appendPackGUID(GetCasterGUID());
            data << uint32(GetId());
            data << uint32(1);
            data << uint32(m_auraName);
            data << uint32(power);                          // power type
            data << (uint32)amount;
            m_target->SendMessageToSet(&data,true);

            int32 gain = m_target->ModifyPower(power,amount);

            if(Unit* pCaster = GetCaster())
                m_target->getHostilRefManager().threatAssist(pCaster, float(gain) * 0.5f, GetSpellProto());
            break;
        }
        case SPELL_AURA_PERIODIC_ENERGIZE:
        {
            // ignore non positive values (can be result apply spellmods to aura damage
            if(m_amount < 0 || GetMiscValue() >= MAX_POWERS)
                return;

            Powers power = Powers(GetMiscValue());

            if(m_target->GetMaxPower(power) == 0)
                return;

            if(GetParentAura()->GetAuraDuration() ==-1 && m_target->GetPower(power)==m_target->GetMaxPower(power))
                return;

            uint32 amount = m_amount;

            WorldPacket data(SMSG_PERIODICAURALOG, (21+16));// we guess size
            data.append(m_target->GetPackGUID());
            data.appendPackGUID(GetCasterGUID());
            data << uint32(GetId());
            data << uint32(1);
            data << uint32(m_auraName);
            data << (uint32)power;                          // power type
            data << (uint32)amount;
            m_target->SendMessageToSet(&data,true);

            sLog.outDetail("PeriodicTick: %u (TypeId: %u) energize %u (TypeId: %u) for %u dmg inflicted by %u",
                GUID_LOPART(GetCasterGUID()), GuidHigh2TypeId(GUID_HIPART(GetCasterGUID())), m_target->GetGUIDLow(), m_target->GetTypeId(), amount, GetId());

            int32 gain = m_target->ModifyPower(power,amount);

            if(Unit* pCaster = GetCaster())
                m_target->getHostilRefManager().threatAssist(pCaster, float(gain) * 0.5f, GetSpellProto());
            break;
        }
        case SPELL_AURA_POWER_BURN_MANA:
        {
            Unit *pCaster = GetCaster();
            if(!pCaster)
                return;

            // Check for immune (not use charges)
            if(m_target->IsImmunedToDamage(GetSpellSchoolMask(GetSpellProto())))
                return;

            int32 pdamage = m_amount > 0 ? m_amount : 0;

            Powers powerType = Powers(GetMiscValue());

            if(!m_target->isAlive() || m_target->getPowerType() != powerType)
                return;

            // resilience reduce mana draining effect at spell crit damage reduction (added in 2.4)
            if (powerType == POWER_MANA && m_target->GetTypeId() == TYPEID_PLAYER)
                pdamage -= ((Player*)m_target)->GetSpellCritDamageReduction(pdamage);

            uint32 gain = uint32(-m_target->ModifyPower(powerType, -pdamage));

            gain = uint32(gain * GetSpellProto()->EffectMultipleValue[GetEffIndex()]);

            SpellEntry const* spellProto = GetSpellProto();
            //maybe has to be sent different to client, but not by SMSG_PERIODICAURALOG
            SpellNonMeleeDamage damageInfo(pCaster, m_target, spellProto->Id, spellProto->SchoolMask);
            //no SpellDamageBonus for burn mana
            pCaster->CalculateSpellDamageTaken(&damageInfo, gain, spellProto);
            pCaster->SendSpellNonMeleeDamageLog(&damageInfo);

            // Set trigger flag
            uint32 procAttacker = PROC_FLAG_ON_DO_PERIODIC;//   | PROC_FLAG_SUCCESSFUL_HARMFUL_SPELL_HIT;
            uint32 procVictim   = PROC_FLAG_ON_TAKE_PERIODIC;// | PROC_FLAG_TAKEN_HARMFUL_SPELL_HIT;
            uint32 procEx       = createProcExtendMask(&damageInfo, SPELL_MISS_NONE);
            if (damageInfo.damage)
                procVictim|=PROC_FLAG_TAKEN_ANY_DAMAGE;

            pCaster->ProcDamageAndSpell(damageInfo.target, procAttacker, procVictim, procEx, damageInfo.damage, BASE_ATTACK, spellProto);

            pCaster->DealSpellDamage(&damageInfo, true);
            break;
        }
        case SPELL_AURA_MOD_REGEN:
        {
            int32 gain = m_target->ModifyHealth(m_amount);
            if (Unit *caster = GetCaster())
                m_target->getHostilRefManager().threatAssist(caster, float(gain) * 0.5f, GetSpellProto());
            break;
        }
        case SPELL_AURA_MOD_POWER_REGEN:
        {
            Powers pt = m_target->getPowerType();
            if(int32(pt) != GetMiscValue())
                return;

            if ( GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED )
            {
                // eating anim
                m_target->HandleEmoteCommand(EMOTE_ONESHOT_EAT);
            }
            else if( GetId() == 20577 )
            {
                // cannibalize anim
                m_target->HandleEmoteCommand(EMOTE_STATE_CANNIBALIZE);
            }

            // Anger Management
            // amount = 1+ 16 = 17 = 3,4*5 = 10,2*5/3
            // so 17 is rounded amount for 5 sec tick grow ~ 1 range grow in 3 sec
            if(pt == POWER_RAGE)
                m_target->ModifyPower(pt, m_amount*3/5);
            break;
        }
        // Here tick dummy auras
        case SPELL_AURA_PERIODIC_DUMMY:
        {
            PeriodicDummyTick();
            break;
        }
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        {
            TriggerSpell();
            break;
        }
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
        {
            TriggerSpellWithValue();
            break;
        }
        default:
            break;
    }
}

void AuraEffect::PeriodicDummyTick()
{
    Unit *caster = GetCaster();
    SpellEntry const* spell = GetSpellProto();
    switch (spell->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        switch (spell->Id)
        {
            // Drink
            case 430:
            case 431:
            case 432:
            case 1133:
            case 1135:
            case 1137:
            case 10250:
            case 22734:
            case 27089:
            case 34291:
            case 43706:
            case 46755:
            case 49472: // Drink Coffee
            case 61830:
            {
                if (m_target->GetTypeId() != TYPEID_PLAYER)
                    return;
                // Search SPELL_AURA_MOD_POWER_REGEN aura for this spell and add bonus
                Unit::AuraEffectList const& aura = m_target->GetAurasByType(SPELL_AURA_MOD_POWER_REGEN);
                for(Unit::AuraEffectList::const_iterator i = aura.begin(); i != aura.end(); ++i)
                {
                    if ((*i)->GetId() == GetId())
                    {
                        (*i)->SetAmount(m_amount);
                        ((Player*)m_target)->UpdateManaRegen();
                        // Disable continue
                        m_isPeriodic = false;
                        return;
                    }
                }
                return;
            }
            // Forsaken Skills
            case 7054:
            {
                // Possibly need cast one of them (but
                // 7038 Forsaken Skill: Swords
                // 7039 Forsaken Skill: Axes
                // 7040 Forsaken Skill: Daggers
                // 7041 Forsaken Skill: Maces
                // 7042 Forsaken Skill: Staves
                // 7043 Forsaken Skill: Bows
                // 7044 Forsaken Skill: Guns
                // 7045 Forsaken Skill: 2H Axes
                // 7046 Forsaken Skill: 2H Maces
                // 7047 Forsaken Skill: 2H Swords
                // 7048 Forsaken Skill: Defense
                // 7049 Forsaken Skill: Fire
                // 7050 Forsaken Skill: Frost
                // 7051 Forsaken Skill: Holy
                // 7053 Forsaken Skill: Shadow
                return;
            }
//        // Panda
//        case 19230: break;
//        // Gossip NPC Periodic - Talk
//        case 33208: break;
//        // Gossip NPC Periodic - Despawn
//        case 33209: break;
//        // Steal Weapon
//        case 36207: break;
//        // Simon Game START timer, (DND)
//        case 39993: break;
//        // Knockdown Fel Cannon: break; The Aggro Burst
//        case 40119: break;
//        // Old Mount Spell
//        case 40154: break;
//        // Magnetic Pull
//        case 40581: break;
//        // Ethereal Ring: break; The Bolt Burst
//        case 40801: break;
//        // Crystal Prison
//        case 40846: break;
//        // Copy Weapon
//        case 41054: break;
//        // Dementia
//        case 41404: break;
//        // Ethereal Ring Visual, Lightning Aura
//        case 41477: break;
//        // Ethereal Ring Visual, Lightning Aura (Fork)
//        case 41525: break;
//        // Ethereal Ring Visual, Lightning Jumper Aura
//        case 41567: break;
//        // No Man's Land
//        case 41955: break;
//        // Headless Horseman - Fire
//        case 42074: break;
//        // Headless Horseman - Visual - Large Fire
//        case 42075: break;
//        // Headless Horseman - Start Fire, Periodic Aura
//        case 42140: break;
//        // Ram Speed Boost
//        case 42152: break;
//        // Headless Horseman - Fires Out Victory Aura
//        case 42235: break;
//        // Pumpkin Life Cycle
//        case 42280: break;
//        // Brewfest Request Chick Chuck Mug Aura
//        case 42537: break;
//        // Squashling
//        case 42596: break;
//        // Headless Horseman Climax, Head: Periodic
//        case 42603: break;
//        // Fire Bomb
//        case 42621: break;
//        // Headless Horseman - Conflagrate, Periodic Aura
//        case 42637: break;
//        // Headless Horseman - Create Pumpkin Treats Aura
//        case 42774: break;
//        // Headless Horseman Climax - Summoning Rhyme Aura
//        case 42879: break;
//        // Tricky Treat
//        case 42919: break;
//        // Giddyup!
//        case 42924: break;
//        // Ram - Trot
//        case 42992: break;
//        // Ram - Canter
//        case 42993: break;
//        // Ram - Gallop
//        case 42994: break;
//        // Ram Level - Neutral
//        case 43310: break;
//        // Headless Horseman - Maniacal Laugh, Maniacal, Delayed 17
//        case 43884: break;
//        // Wretched!
//        case 43963: break;
//        // Headless Horseman - Maniacal Laugh, Maniacal, other, Delayed 17
//        case 44000: break;
//        // Energy Feedback
//        case 44328: break;
//        // Romantic Picnic
//        case 45102: break;
//        // Romantic Picnic
//        case 45123: break;
//        // Looking for Love
//        case 45124: break;
//        // Kite - Lightning Strike Kite Aura
//        case 45197: break;
//        // Rocket Chicken
//        case 45202: break;
//        // Copy Offhand Weapon
//        case 45205: break;
//        // Upper Deck - Kite - Lightning Periodic Aura
//        case 45207: break;
//        // Kite -Sky  Lightning Strike Kite Aura
//        case 45251: break;
//        // Ribbon Pole Dancer Check Aura
//        case 45390: break;
//        // Holiday - Midsummer, Ribbon Pole Periodic Visual
//        case 45406: break;
//        // Parachute
//        case 45472: break;
//        // Alliance Flag, Extra Damage Debuff
//        case 45898: break;
//        // Horde Flag, Extra Damage Debuff
//        case 45899: break;
//        // Ahune - Summoning Rhyme Aura
//        case 45926: break;
//        // Ahune - Slippery Floor
//        case 45945: break;
//        // Ahune's Shield
//        case 45954: break;
//        // Nether Vapor Lightning
//        case 45960: break;
//        // Darkness
//        case 45996: break;
//        // Summon Blood Elves Periodic
//        case 46041: break;
//        // Transform Visual Missile Periodic
//        case 46205: break;
//        // Find Opening Beam End
//        case 46333: break;
//        // Ice Spear Control Aura
//        case 46371: break;
//        // Hailstone Chill
//        case 46458: break;
//        // Hailstone Chill, Internal
//        case 46465: break;
//        // Chill, Internal Shifter
//        case 46549: break;
//        // Summon Ice Spear Knockback Delayer
//        case 46878: break;
//        // Burninate Effect
//        case 47214: break;
//        // Fizzcrank Practice Parachute
//        case 47228: break;
//        // Send Mug Control Aura
//        case 47369: break;
//        // Direbrew's Disarm (precast)
//        case 47407: break;
//        // Mole Machine Port Schedule
//        case 47489: break;
//        case 47941: break; // Crystal Spike
//        case 48200: break; // Healer Aura
//        case 48630: break; // Summon Gauntlet Mobs Periodic
//        case 49313: break; // Proximity Mine Area Aura
//        // Mole Machine Portal Schedule
//        case 49466: break;
//        case 49555: break; // Corpse Explode
//        case 49592: break; // Temporal Rift
//        case 49957: break; // Cutting Laser
//        case 50085: break; // Slow Fall
//        // Listening to Music
//        case 50493: break;
//        // Love Rocket Barrage
//        case 50530: break;
// Exist more after, need add later
            default:
                break;
        }
        break;
        case SPELLFAMILY_MAGE:
        {
            // Mirror Image
//            if (spell->Id == 55342)
//                return;
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            // Armored to the Teeth
            if (spell->SpellIconID == 3516)
            {
                // Increases your attack power by $s1 for every $s2 armor value you have.
                // Calculate AP bonus (from 1 efect of this spell)
                int32 apBonus = m_amount * m_target->GetArmor() / m_target->CalculateSpellDamage(spell, 1, spell->EffectBasePoints[1], m_target);
                m_target->CastCustomSpell(m_target, 61217, &apBonus, &apBonus, 0, true, 0, this);
                return;
            }
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            switch (spell->Id)
            {
                // Frenzied Regeneration
                case 22842:
                {
                    // Converts up to 10 rage per second into health for $d.  Each point of rage is converted into ${$m2/10}.1% of max health.
                    // Should be manauser
                    if (m_target->getPowerType()!=POWER_RAGE)
                        return;
                    uint32 rage = m_target->GetPower(POWER_RAGE);
                    // Nothing todo
                    if (rage == 0)
                        return;
                    int32 mod = (rage < 100) ? rage : 100;
                    int32 points = m_target->CalculateSpellDamage(spell, 1, spell->EffectBasePoints[1], m_target);
                    int32 regen = m_target->GetMaxHealth() * (mod * points / 10) / 1000;
                    m_target->CastCustomSpell(m_target, 22845, &regen, 0, 0, true, 0, this);
                    m_target->SetPower(POWER_RAGE, rage-mod);
                    return;
                }
                // Force of Nature
                case 33831:
                    return;
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            switch (spell->Id)
            {
                // Killing Spree
//                case 51690: break;
                // Overkill
                case 58428:
                    if (!m_target->HasAuraType(SPELL_AURA_MOD_STEALTH))
                        m_target->RemoveAurasDueToSpell(58427);
                    break;
                // Master of Subtlety
                case 31666:
                    if (!m_target->HasAuraType(SPELL_AURA_MOD_STEALTH))
                        m_target->RemoveAurasDueToSpell(31665);
                    break;
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Explosive Shot
            if (spell->SpellFamilyFlags[1] & 0x80000000)
            {
                if (!caster)
                    return;
                int32 damage = m_amount;
                caster->CastCustomSpell(m_target, 56298, &damage, 0, 0, true, 0, this);
                return;
            }
            switch (spell->Id)
            {
                // Harpooner's Mark
                // case 40084:
                //    return;
                // Feeding Frenzy Rank 1
                case 53511:
                    if ( m_target->GetHealth() * 100 < m_target->GetMaxHealth() * 35 )
                        m_target->CastSpell(m_target, 60096, true, 0, this);
                    return;
                // Feeding Frenzy Rank 2
                case 53512:
                    if ( m_target->GetHealth() * 100 < m_target->GetMaxHealth() * 35 )
                        m_target->CastSpell(m_target, 60097, true, 0, this);
                    return;
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            // Astral Shift
            if (spell->Id == 52179)
            {
                // Periodic need for remove visual on stun/fear/silence lost
                if (!(m_target->GetUInt32Value(UNIT_FIELD_FLAGS)&(UNIT_FLAG_STUNNED|UNIT_FLAG_FLEEING|UNIT_FLAG_SILENCED)))
                    m_target->RemoveAurasDueToSpell(52179);
                return;
            }
            break;
        }
        case SPELLFAMILY_DEATHKNIGHT:
        {
            // Death and Decay
            if (spell->SpellFamilyFlags[0] & 0x20)
            {
                if (caster)
                    caster->CastCustomSpell(m_target, 52212, &m_amount, NULL, NULL, true, 0, this);
                return;
            }
            // Raise Dead
//            if (spell->SpellFamilyFlags & 0x0000000000001000LL)
//                return;
            // Chains of Ice
            if (spell->SpellFamilyFlags[1] & 0x00004000)
            {
                // Get 0 effect aura
                AuraEffect *slow = m_target->GetAuraEffect(GetId(), 0);
                if (slow)
                {
                    slow->ApplyModifier(false);;
                    slow->SetAmount(GetAmount()*2);
                    if (slow->GetAmount() > 0) slow->SetAmount(0);
                    slow->ApplyModifier(true);
                }
                return;
            }
            // Summon Gargoyle
//            if (spell->SpellFamilyFlags & 0x0000008000000000LL)
//                return;
            // Death Rune Mastery
//            if (spell->SpellFamilyFlags & 0x0000000000004000LL)
//                return;
            // Bladed Armor
            if (spell->SpellIconID == 2653)
            {
                // Increases your attack power by $s1 for every $s2 armor value you have.
                // Calculate AP bonus (from 1 efect of this spell)
                int32 apBonus = m_amount * m_target->GetArmor() / m_target->CalculateSpellDamage(spell, 1, spell->EffectBasePoints[1], m_target);
                m_target->CastCustomSpell(m_target, 61217, &apBonus, &apBonus, 0, true, 0, this);
                return;
            }
            // Reaping
//            if (spell->SpellIconID == 22)
//                return;
            // Blood of the North
//            if (spell->SpellIconID == 30412)
//                return;
            break;
        }
        default:
            break;
    }
}

void AuraEffect::HandlePreventFleeing(bool apply, bool Real)
{
    if(!Real)
        return;

    Unit::AuraEffectList const& fearAuras = m_target->GetAurasByType(SPELL_AURA_MOD_FEAR);
    if( !fearAuras.empty() )
    {
        m_target->SetControlled(!apply, UNIT_STAT_FLEEING);
        /*if (apply)
            m_target->SetFeared(false, fearAuras.front()->GetCasterGUID());
        else
            m_target->SetFeared(true);*/
    }
}

void AuraEffect::HandleManaShield(bool apply, bool Real)
{
    if(!Real)
        return;

    // prevent double apply bonuses
    if(apply && (m_target->GetTypeId()!=TYPEID_PLAYER || !((Player*)m_target)->GetSession()->PlayerLoading()))
    {
        if(Unit* caster = GetCaster())
        {
            float DoneActualBenefit = 0.0f;
            switch(m_spellProto->SpellFamilyName)
            {
                case SPELLFAMILY_MAGE:
                    if(m_spellProto->SpellFamilyFlags[0] & 0x8000)
                    {
                        // Mana Shield
                        // +50% from +spd bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonus(GetSpellSchoolMask(m_spellProto)) * 0.5f;
                        break;
                    }
                    break;
                default:
                    break;
            }

            DoneActualBenefit *= caster->CalculateLevelPenalty(GetSpellProto());

            m_amount += (int32)DoneActualBenefit;
        }
    }
}

void AuraEffect::HandleArenaPreparation(bool apply, bool Real)
{
    if(!Real)
        return;

    if(apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION);
}

void AuraEffect::HandleAuraControlVehicle(bool apply, bool Real)
{
    if(!Real)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if(Pet *pet = ((Player*)m_target)->GetPet())
        pet->Remove(PET_SAVE_AS_CURRENT);

    //WorldPacket data(SMSG_ON_CANCEL_EXPECTED_RIDE_VEHICLE_AURA, 0);
    //((Player*)m_target)->GetSession()->SendPacket(&data);
}

void AuraEffect::HandleAuraConvertRune(bool apply, bool Real)
{
    if(!Real)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *plr = (Player*)m_target;

    if(plr->getClass() != CLASS_DEATH_KNIGHT)
        return;

    // how to determine what rune need to be converted?
    for(uint32 i = 0; i < MAX_RUNES; ++i)
    {
        if(apply)
        {
            if(!plr->GetRuneCooldown(i))
            {
                plr->ConvertRune(i, GetSpellProto()->EffectMiscValueB[m_effIndex]);
                break;
            }
        }
        else
        {
            if(plr->GetCurrentRune(i) == GetSpellProto()->EffectMiscValueB[m_effIndex])
            {
                plr->ConvertRune(i, plr->GetBaseRune(i));
                break;
            }
        }
    }
}

// Control Auras

void AuraEffect::HandleAuraModStun(bool apply, bool Real)
{
    if(!Real)
        return;

    m_target->SetControlled(apply, UNIT_STAT_STUNNED);
}

void AuraEffect::HandleAuraModRoot(bool apply, bool Real)
{
    if(!Real)
        return;

    m_target->SetControlled(apply, UNIT_STAT_ROOT);
}

// Charm Auras

void AuraEffect::HandleModPossess(bool apply, bool Real)
{
    if(!Real)
        return;

    Unit* caster = GetCaster();
    if(caster && caster->GetTypeId() == TYPEID_UNIT)
    {
        HandleModCharm(apply, Real);
        return;
    }

    if(apply)
    {
        if(m_target->getLevel() > m_amount)
            return;

        m_target->SetCharmedOrPossessedBy(caster, true);
    }
    else
        m_target->RemoveCharmedOrPossessedBy(caster);
}

void AuraEffect::HandleModPossessPet(bool apply, bool Real)
{
    if(!Real)
        return;

    Unit* caster = GetCaster();
    if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if(apply)
    {
        if(caster->GetGuardianPet() != m_target)
            return;

        m_target->SetCharmedOrPossessedBy(caster, true);
    }
    else
    {
        m_target->RemoveCharmedOrPossessedBy(caster);

        // Reinitialize the pet bar and make the pet come back to the owner
        ((Player*)caster)->PetSpellInitialize();
        if(!m_target->getVictim())
        {
            m_target->GetMotionMaster()->MoveFollow(caster, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
            m_target->GetCharmInfo()->SetCommandState(COMMAND_FOLLOW);
        }
    }
}

void AuraEffect::HandleModCharm(bool apply, bool Real)
{
    if(!Real)
        return;

    Unit* caster = GetCaster();

    if(apply)
    {
        if(int32(m_target->getLevel()) > m_amount)
            return;

        m_target->SetCharmedOrPossessedBy(caster, false);
    }
    else
        m_target->RemoveCharmedOrPossessedBy(caster);
}

void AuraEffect::HandlePhase(bool apply, bool Real)
{
    if(!Real)
        return;

    // always non stackable
    if(apply)
    {
        m_target->RemoveAurasByType(SPELL_AURA_PHASE, NULL, GetParentAura());
    }

    // no-phase is also phase state so same code for apply and remove

    // phase auras normally not expected at BG but anyway better check
    if(m_target->GetTypeId()==TYPEID_PLAYER)
    {
        // drop flag at invisible in bg
        if(((Player*)m_target)->InBattleGround())
            if(BattleGround *bg = ((Player*)m_target)->GetBattleGround())
                bg->EventPlayerDroppedFlag((Player*)m_target);

        // GM-mode have mask 0xFFFFFFFF
        if(!((Player*)m_target)->isGameMaster())
            m_target->SetPhaseMask(apply ? GetMiscValue() : PHASEMASK_NORMAL,false);

        ((Player*)m_target)->GetSession()->SendSetPhaseShift(apply ? GetMiscValue() : PHASEMASK_NORMAL);

        if(GetEffIndex()==0)
        {
            SpellAreaForAreaMapBounds saBounds = spellmgr.GetSpellAreaForAuraMapBounds(GetId());
            if(saBounds.first != saBounds.second)
            {
                uint32 zone, area;
                m_target->GetZoneAndAreaId(zone,area);

                for(SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
                {
                    // some auras remove at aura remove
                    if(!itr->second->IsFitToRequirements((Player*)m_target,zone,area))
                        m_target->RemoveAurasDueToSpell(itr->second->spellId);
                    // some auras applied at aura apply
                    else if(itr->second->autocast)
                    {
                        if( !m_target->HasAura(itr->second->spellId,0) )
                            m_target->CastSpell(m_target,itr->second->spellId,true);
                    }
                }
            }
        }
    }
    else
        m_target->SetPhaseMask(apply ? GetMiscValue() : PHASEMASK_NORMAL,false);

    // need triggering visibility update base at phase update of not GM invisible (other GMs anyway see in any phases)
    if(m_target->GetVisibility()!=VISIBILITY_OFF)
        m_target->SetVisibility(m_target->GetVisibility());
}

void Aura::UnregisterSingleCastAura()
{
    if (IsSingleTarget())
    {
        Unit* caster = NULL;
        caster = GetCaster();
        if(caster)
        {
            caster->GetSingleCastAuras().remove(this);
        }
        else
        {
            sLog.outError("Couldn't find the caster of the single target aura, may crash later!");
            assert(false);
        }
        m_isSingleTargetAura = false;
    }
}
