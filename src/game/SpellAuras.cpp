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
#include "Vehicle.h"
#include "CellImpl.h"

#define Aura AuraEffect
pAuraHandler AuraHandler[TOTAL_AURAS]=
{
    &Aura::HandleNULL,                                      //  0 SPELL_AURA_NONE
    &Aura::HandleBindSight,                                 //  1 SPELL_AURA_BIND_SIGHT
    &Aura::HandleModPossess,                                //  2 SPELL_AURA_MOD_POSSESS
    &Aura::HandlePeriodicDamage,                            //  3 SPELL_AURA_PERIODIC_DAMAGE
    &Aura::HandleAuraDummy,                                 //  4 SPELL_AURA_DUMMY
    &Aura::HandleModConfuse,                                //  5 SPELL_AURA_MOD_CONFUSE
    &Aura::HandleModCharm,                                  //  6 SPELL_AURA_MOD_CHARM
    &Aura::HandleModFear,                                   //  7 SPELL_AURA_MOD_FEAR
    &Aura::HandlePeriodicHeal,                              //  8 SPELL_AURA_PERIODIC_HEAL
    &Aura::HandleModAttackSpeed,                            //  9 SPELL_AURA_MOD_ATTACKSPEED
    &Aura::HandleModThreat,                                 // 10 SPELL_AURA_MOD_THREAT
    &Aura::HandleModTaunt,                                  // 11 SPELL_AURA_MOD_TAUNT
    &Aura::HandleAuraModStun,                               // 12 SPELL_AURA_MOD_STUN
    &Aura::HandleModDamageDone,                             // 13 SPELL_AURA_MOD_DAMAGE_DONE
    &Aura::HandleNoImmediateEffect,                         // 14 SPELL_AURA_MOD_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &Aura::HandleNoImmediateEffect,                         // 15 SPELL_AURA_DAMAGE_SHIELD    implemented in Unit::DoAttackDamage
    &Aura::HandleModStealth,                                // 16 SPELL_AURA_MOD_STEALTH
    &Aura::HandleNoImmediateEffect,                         // 17 SPELL_AURA_MOD_STEALTH_DETECT
    &Aura::HandleInvisibility,                              // 18 SPELL_AURA_MOD_INVISIBILITY
    &Aura::HandleInvisibilityDetect,                        // 19 SPELL_AURA_MOD_INVISIBILITY_DETECTION
    &Aura::HandleAuraModTotalHealthPercentRegen,            // 20 SPELL_AURA_OBS_MOD_HEALTH
    &Aura::HandleAuraModTotalEnergyPercentRegen,            // 21 SPELL_AURA_OBS_MOD_POWER
    &Aura::HandleAuraModResistance,                         // 22 SPELL_AURA_MOD_RESISTANCE
    &Aura::HandlePeriodicTriggerSpell,                      // 23 SPELL_AURA_PERIODIC_TRIGGER_SPELL
    &Aura::HandlePeriodicEnergize,                          // 24 SPELL_AURA_PERIODIC_ENERGIZE
    &Aura::HandleAuraModPacify,                             // 25 SPELL_AURA_MOD_PACIFY
    &Aura::HandleAuraModRoot,                               // 26 SPELL_AURA_MOD_ROOT
    &Aura::HandleAuraModSilence,                            // 27 SPELL_AURA_MOD_SILENCE
    &Aura::HandleNoImmediateEffect,                         // 28 SPELL_AURA_REFLECT_SPELLS        implement in Unit::SpellHitResult
    &Aura::HandleAuraModStat,                               // 29 SPELL_AURA_MOD_STAT
    &Aura::HandleAuraModSkill,                              // 30 SPELL_AURA_MOD_SKILL
    &Aura::HandleAuraModIncreaseSpeed,                      // 31 SPELL_AURA_MOD_INCREASE_SPEED
    &Aura::HandleAuraModIncreaseMountedSpeed,               // 32 SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED
    &Aura::HandleAuraModDecreaseSpeed,                      // 33 SPELL_AURA_MOD_DECREASE_SPEED
    &Aura::HandleAuraModIncreaseHealth,                     // 34 SPELL_AURA_MOD_INCREASE_HEALTH
    &Aura::HandleAuraModIncreaseEnergy,                     // 35 SPELL_AURA_MOD_INCREASE_ENERGY
    &Aura::HandleAuraModShapeshift,                         // 36 SPELL_AURA_MOD_SHAPESHIFT
    &Aura::HandleAuraModEffectImmunity,                     // 37 SPELL_AURA_EFFECT_IMMUNITY
    &Aura::HandleAuraModStateImmunity,                      // 38 SPELL_AURA_STATE_IMMUNITY
    &Aura::HandleAuraModSchoolImmunity,                     // 39 SPELL_AURA_SCHOOL_IMMUNITY
    &Aura::HandleAuraModDmgImmunity,                        // 40 SPELL_AURA_DAMAGE_IMMUNITY
    &Aura::HandleAuraModDispelImmunity,                     // 41 SPELL_AURA_DISPEL_IMMUNITY
    &Aura::HandleAuraProcTriggerSpell,                      // 42 SPELL_AURA_PROC_TRIGGER_SPELL  implemented in Unit::ProcDamageAndSpellFor and Unit::HandleProcTriggerSpell
    &Aura::HandleNoImmediateEffect,                         // 43 SPELL_AURA_PROC_TRIGGER_DAMAGE implemented in Unit::ProcDamageAndSpellFor
    &Aura::HandleAuraTrackCreatures,                        // 44 SPELL_AURA_TRACK_CREATURES
    &Aura::HandleAuraTrackResources,                        // 45 SPELL_AURA_TRACK_RESOURCES
    &Aura::HandleNULL,                                      // 46 SPELL_AURA_46 (used in test spells 54054 and 54058, and spell 48050) (3.0.8a)
    &Aura::HandleAuraModParryPercent,                       // 47 SPELL_AURA_MOD_PARRY_PERCENT
    &Aura::HandleNULL,                                      // 48 SPELL_AURA_48 spell Napalm (area damage spell with additional delayed damage effect)
    &Aura::HandleAuraModDodgePercent,                       // 49 SPELL_AURA_MOD_DODGE_PERCENT
    &Aura::HandleNoImmediateEffect,                         // 50 SPELL_AURA_MOD_CRITICAL_HEALING_AMOUNT implemented in Unit::SpellCriticalHealingBonus
    &Aura::HandleAuraModBlockPercent,                       // 51 SPELL_AURA_MOD_BLOCK_PERCENT
    &Aura::HandleAuraModWeaponCritPercent,                  // 52 SPELL_AURA_MOD_WEAPON_CRIT_PERCENT
    &Aura::HandlePeriodicLeech,                             // 53 SPELL_AURA_PERIODIC_LEECH
    &Aura::HandleModHitChance,                              // 54 SPELL_AURA_MOD_HIT_CHANCE
    &Aura::HandleModSpellHitChance,                         // 55 SPELL_AURA_MOD_SPELL_HIT_CHANCE
    &Aura::HandleAuraTransform,                             // 56 SPELL_AURA_TRANSFORM
    &Aura::HandleModSpellCritChance,                        // 57 SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    &Aura::HandleAuraModIncreaseSwimSpeed,                  // 58 SPELL_AURA_MOD_INCREASE_SWIM_SPEED
    &Aura::HandleNoImmediateEffect,                         // 59 SPELL_AURA_MOD_DAMAGE_DONE_CREATURE implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &Aura::HandleAuraModPacifyAndSilence,                   // 60 SPELL_AURA_MOD_PACIFY_SILENCE
    &Aura::HandleAuraModScale,                              // 61 SPELL_AURA_MOD_SCALE
    &Aura::HandlePeriodicHealthFunnel,                      // 62 SPELL_AURA_PERIODIC_HEALTH_FUNNEL
    &Aura::HandleNULL,                                      // 63 unused (3.0.8a) old SPELL_AURA_PERIODIC_MANA_FUNNEL
    &Aura::HandlePeriodicManaLeech,                         // 64 SPELL_AURA_PERIODIC_MANA_LEECH
    &Aura::HandleModCastingSpeed,                           // 65 SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK
    &Aura::HandleFeignDeath,                                // 66 SPELL_AURA_FEIGN_DEATH
    &Aura::HandleAuraModDisarm,                             // 67 SPELL_AURA_MOD_DISARM
    &Aura::HandleAuraModStalked,                            // 68 SPELL_AURA_MOD_STALKED
    &Aura::HandleSchoolAbsorb,                              // 69 SPELL_AURA_SCHOOL_ABSORB implemented in Unit::CalcAbsorbResist
    &Aura::HandleUnused,                                    // 70 SPELL_AURA_EXTRA_ATTACKS      Useless, used by only one spell that has only visual effect
    &Aura::HandleModSpellCritChanceShool,                   // 71 SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    &Aura::HandleModPowerCostPCT,                           // 72 SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT
    &Aura::HandleModPowerCost,                              // 73 SPELL_AURA_MOD_POWER_COST_SCHOOL
    &Aura::HandleNoImmediateEffect,                         // 74 SPELL_AURA_REFLECT_SPELLS_SCHOOL  implemented in Unit::SpellHitResult
    &Aura::HandleNoImmediateEffect,                         // 75 SPELL_AURA_MOD_LANGUAGE
    &Aura::HandleFarSight,                                  // 76 SPELL_AURA_FAR_SIGHT
    &Aura::HandleModMechanicImmunity,                       // 77 SPELL_AURA_MECHANIC_IMMUNITY
    &Aura::HandleAuraMounted,                               // 78 SPELL_AURA_MOUNTED
    &Aura::HandleModDamagePercentDone,                      // 79 SPELL_AURA_MOD_DAMAGE_PERCENT_DONE
    &Aura::HandleModPercentStat,                            // 80 SPELL_AURA_MOD_PERCENT_STAT
    &Aura::HandleNoImmediateEffect,                         // 81 SPELL_AURA_SPLIT_DAMAGE_PCT
    &Aura::HandleWaterBreathing,                            // 82 SPELL_AURA_WATER_BREATHING
    &Aura::HandleModBaseResistance,                         // 83 SPELL_AURA_MOD_BASE_RESISTANCE
    &Aura::HandleModRegen,                                  // 84 SPELL_AURA_MOD_REGEN
    &Aura::HandleModPowerRegen,                             // 85 SPELL_AURA_MOD_POWER_REGEN
    &Aura::HandleChannelDeathItem,                          // 86 SPELL_AURA_CHANNEL_DEATH_ITEM
    &Aura::HandleNoImmediateEffect,                         // 87 SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonus
    &Aura::HandleNoImmediateEffect,                         // 88 SPELL_AURA_MOD_HEALTH_REGEN_PERCENT
    &Aura::HandlePeriodicDamagePCT,                         // 89 SPELL_AURA_PERIODIC_DAMAGE_PERCENT
    &Aura::HandleUnused,                                    // 90 unused (3.0.8a) old SPELL_AURA_MOD_RESIST_CHANCE
    &Aura::HandleNoImmediateEffect,                         // 91 SPELL_AURA_MOD_DETECT_RANGE implemented in Creature::GetAttackDistance
    &Aura::HandlePreventFleeing,                            // 92 SPELL_AURA_PREVENTS_FLEEING
    &Aura::HandleModUnattackable,                           // 93 SPELL_AURA_MOD_UNATTACKABLE
    &Aura::HandleNoImmediateEffect,                         // 94 SPELL_AURA_INTERRUPT_REGEN implemented in Player::RegenerateAll
    &Aura::HandleAuraGhost,                                 // 95 SPELL_AURA_GHOST
    &Aura::HandleNoImmediateEffect,                         // 96 SPELL_AURA_SPELL_MAGNET implemented in Unit::SelectMagnetTarget
    &Aura::HandleNoImmediateEffect,                         // 97 SPELL_AURA_MANA_SHIELD implemented in Unit::CalcAbsorbResist
    &Aura::HandleAuraModSkill,                              // 98 SPELL_AURA_MOD_SKILL_TALENT
    &Aura::HandleAuraModAttackPower,                        // 99 SPELL_AURA_MOD_ATTACK_POWER
    &Aura::HandleUnused,                                    //100 SPELL_AURA_AURAS_VISIBLE obsolete? all player can see all auras now, but still have spells including GM-spell
    &Aura::HandleModResistancePercent,                      //101 SPELL_AURA_MOD_RESISTANCE_PCT
    &Aura::HandleNoImmediateEffect,                         //102 SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonus
    &Aura::HandleAuraModTotalThreat,                        //103 SPELL_AURA_MOD_TOTAL_THREAT
    &Aura::HandleAuraWaterWalk,                             //104 SPELL_AURA_WATER_WALK
    &Aura::HandleAuraFeatherFall,                           //105 SPELL_AURA_FEATHER_FALL
    &Aura::HandleAuraHover,                                 //106 SPELL_AURA_HOVER
    &Aura::HandleAddModifier,                               //107 SPELL_AURA_ADD_FLAT_MODIFIER
    &Aura::HandleAddModifier,                               //108 SPELL_AURA_ADD_PCT_MODIFIER
    &Aura::HandleNoImmediateEffect,                         //109 SPELL_AURA_ADD_TARGET_TRIGGER
    &Aura::HandleModPowerRegenPCT,                          //110 SPELL_AURA_MOD_POWER_REGEN_PERCENT
    &Aura::HandleNoImmediateEffect,                         //111 SPELL_AURA_ADD_CASTER_HIT_TRIGGER implemented in Unit::SelectMagnetTarget
    &Aura::HandleNoImmediateEffect,                         //112 SPELL_AURA_OVERRIDE_CLASS_SCRIPTS
    &Aura::HandleNoImmediateEffect,                         //113 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus
    &Aura::HandleNoImmediateEffect,                         //114 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonus
    &Aura::HandleNoImmediateEffect,                         //115 SPELL_AURA_MOD_HEALING                 implemented in Unit::SpellBaseHealingBonusForVictim
    &Aura::HandleNoImmediateEffect,                         //116 SPELL_AURA_MOD_REGEN_DURING_COMBAT
    &Aura::HandleNoImmediateEffect,                         //117 SPELL_AURA_MOD_MECHANIC_RESISTANCE     implemented in Unit::MagicSpellHitResult
    &Aura::HandleNoImmediateEffect,                         //118 SPELL_AURA_MOD_HEALING_PCT             implemented in Unit::SpellHealingBonus
    &Aura::HandleUnused,                                    //119 unused (3.0.8a) old SPELL_AURA_SHARE_PET_TRACKING
    &Aura::HandleAuraUntrackable,                           //120 SPELL_AURA_UNTRACKABLE
    &Aura::HandleAuraEmpathy,                               //121 SPELL_AURA_EMPATHY
    &Aura::HandleModOffhandDamagePercent,                   //122 SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT
    &Aura::HandleModTargetResistance,                       //123 SPELL_AURA_MOD_TARGET_RESISTANCE
    &Aura::HandleAuraModRangedAttackPower,                  //124 SPELL_AURA_MOD_RANGED_ATTACK_POWER
    &Aura::HandleNoImmediateEffect,                         //125 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus
    &Aura::HandleNoImmediateEffect,                         //126 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonus
    &Aura::HandleNoImmediateEffect,                         //127 SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonus
    &Aura::HandleModPossessPet,                             //128 SPELL_AURA_MOD_POSSESS_PET
    &Aura::HandleAuraModIncreaseSpeed,                      //129 SPELL_AURA_MOD_SPEED_ALWAYS
    &Aura::HandleAuraModIncreaseMountedSpeed,               //130 SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS
    &Aura::HandleNoImmediateEffect,                         //131 SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonus
    &Aura::HandleAuraModIncreaseEnergyPercent,              //132 SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT
    &Aura::HandleAuraModIncreaseHealthPercent,              //133 SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT
    &Aura::HandleAuraModRegenInterrupt,                     //134 SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
    &Aura::HandleModHealingDone,                            //135 SPELL_AURA_MOD_HEALING_DONE
    &Aura::HandleNoImmediateEffect,                         //136 SPELL_AURA_MOD_HEALING_DONE_PERCENT   implemented in Unit::SpellHealingBonus
    &Aura::HandleModTotalPercentStat,                       //137 SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE
    &Aura::HandleHaste,                                     //138 SPELL_AURA_MOD_HASTE
    &Aura::HandleForceReaction,                             //139 SPELL_AURA_FORCE_REACTION
    &Aura::HandleAuraModRangedHaste,                        //140 SPELL_AURA_MOD_RANGED_HASTE
    &Aura::HandleRangedAmmoHaste,                           //141 SPELL_AURA_MOD_RANGED_AMMO_HASTE
    &Aura::HandleAuraModBaseResistancePCT,                  //142 SPELL_AURA_MOD_BASE_RESISTANCE_PCT
    &Aura::HandleAuraModResistanceExclusive,                //143 SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE
    &Aura::HandleNoImmediateEffect,                         //144 SPELL_AURA_SAFE_FALL                         implemented in WorldSession::HandleMovementOpcodes
    &Aura::HandleAuraModPetTalentsPoints,                   //145 SPELL_AURA_MOD_PET_TALENT_POINTS
    &Aura::HandleNoImmediateEffect,                         //146 SPELL_AURA_ALLOW_TAME_PET_TYPE
    &Aura::HandleModStateImmunityMask,                      //147 SPELL_AURA_MECHANIC_IMMUNITY_MASK
    &Aura::HandleAuraRetainComboPoints,                     //148 SPELL_AURA_RETAIN_COMBO_POINTS
    &Aura::HandleNoImmediateEffect,                         //149 SPELL_AURA_REDUCE_PUSHBACK
    &Aura::HandleShieldBlockValue,                          //150 SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT
    &Aura::HandleAuraTrackStealthed,                        //151 SPELL_AURA_TRACK_STEALTHED
    &Aura::HandleNoImmediateEffect,                         //152 SPELL_AURA_MOD_DETECTED_RANGE implemented in Creature::GetAttackDistance
    &Aura::HandleNoImmediateEffect,                         //153 SPELL_AURA_SPLIT_DAMAGE_FLAT
    &Aura::HandleNoImmediateEffect,                         //154 SPELL_AURA_MOD_STEALTH_LEVEL
    &Aura::HandleNoImmediateEffect,                         //155 SPELL_AURA_MOD_WATER_BREATHING
    &Aura::HandleNoImmediateEffect,                         //156 SPELL_AURA_MOD_REPUTATION_GAIN
    &Aura::HandleNULL,                                      //157 SPELL_AURA_PET_DAMAGE_MULTI
    &Aura::HandleShieldBlockValue,                          //158 SPELL_AURA_MOD_SHIELD_BLOCKVALUE
    &Aura::HandleNoImmediateEffect,                         //159 SPELL_AURA_NO_PVP_CREDIT      only for Honorless Target spell
    &Aura::HandleNoImmediateEffect,                         //160 SPELL_AURA_MOD_AOE_AVOIDANCE                 implemented in Unit::MagicSpellHitResult
    &Aura::HandleNoImmediateEffect,                         //161 SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT
    &Aura::HandleAuraPowerBurn,                             //162 SPELL_AURA_POWER_BURN_MANA
    &Aura::HandleNoImmediateEffect,                         //163 SPELL_AURA_MOD_CRIT_DAMAGE_BONUS_MELEE
    &Aura::HandleUnused,                                    //164 unused (3.0.8a), only one test spell
    &Aura::HandleNoImmediateEffect,                         //165 SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonus
    &Aura::HandleAuraModAttackPowerPercent,                 //166 SPELL_AURA_MOD_ATTACK_POWER_PCT
    &Aura::HandleAuraModRangedAttackPowerPercent,           //167 SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT
    &Aura::HandleNoImmediateEffect,                         //168 SPELL_AURA_MOD_DAMAGE_DONE_VERSUS            implemented in Unit::SpellDamageBonus, Unit::MeleeDamageBonus
    &Aura::HandleNoImmediateEffect,                         //169 SPELL_AURA_MOD_CRIT_PERCENT_VERSUS           implemented in Unit::DealDamageBySchool, Unit::DoAttackDamage, Unit::SpellCriticalBonus
    &Aura::HandleNULL,                                      //170 SPELL_AURA_DETECT_AMORE       various spells that change visual of units for aura target (clientside?)
    &Aura::HandleAuraModIncreaseSpeed,                      //171 SPELL_AURA_MOD_SPEED_NOT_STACK
    &Aura::HandleAuraModIncreaseMountedSpeed,               //172 SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK
    &Aura::HandleUnused,                                    //173 unused (3.0.8a) no spells, old SPELL_AURA_ALLOW_CHAMPION_SPELLS  only for Proclaim Champion spell
    &Aura::HandleModSpellDamagePercentFromStat,             //174 SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT  implemented in Unit::SpellBaseDamageBonus
    &Aura::HandleModSpellHealingPercentFromStat,            //175 SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT implemented in Unit::SpellBaseHealingBonus
    &Aura::HandleSpiritOfRedemption,                        //176 SPELL_AURA_SPIRIT_OF_REDEMPTION   only for Spirit of Redemption spell, die at aura end
    &Aura::HandleCharmConvert,                              //177 SPELL_AURA_AOE_CHARM
    &Aura::HandleNoImmediateEffect,                         //178 SPELL_AURA_MOD_DEBUFF_RESISTANCE          implemented in Unit::MagicSpellHitResult
    &Aura::HandleNoImmediateEffect,                         //179 SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE implemented in Unit::SpellCriticalBonus
    &Aura::HandleNoImmediateEffect,                         //180 SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS   implemented in Unit::SpellDamageBonus
    &Aura::HandleUnused,                                    //181 unused (3.0.8a) old SPELL_AURA_MOD_FLAT_SPELL_CRIT_DAMAGE_VERSUS
    &Aura::HandleAuraModResistenceOfStatPercent,            //182 SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT
    &Aura::HandleNULL,                                      //183 SPELL_AURA_MOD_CRITICAL_THREAT only used in 28746 - miscvalue - spell school
    &Aura::HandleNoImmediateEffect,                         //184 SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE  implemented in Unit::RollMeleeOutcomeAgainst
    &Aura::HandleNoImmediateEffect,                         //185 SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE implemented in Unit::RollMeleeOutcomeAgainst
    &Aura::HandleNoImmediateEffect,                         //186 SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE  implemented in Unit::MagicSpellHitResult
    &Aura::HandleNoImmediateEffect,                         //187 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE  implemented in Unit::GetUnitCriticalChance
    &Aura::HandleNoImmediateEffect,                         //188 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE implemented in Unit::GetUnitCriticalChance
    &Aura::HandleModRating,                                 //189 SPELL_AURA_MOD_RATING
    &Aura::HandleNoImmediateEffect,                         //190 SPELL_AURA_MOD_FACTION_REPUTATION_GAIN     implemented in Player::CalculateReputationGain
    &Aura::HandleAuraModUseNormalSpeed,                     //191 SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED
    &Aura::HandleModMeleeRangedSpeedPct,                    //192 SPELL_AURA_HASTE_MELEE
    &Aura::HandleModCombatSpeedPct,                         //193 SPELL_AURA_MELEE_SLOW (in fact combat (any type attack) speed pct)
    &Aura::HandleNoImmediateEffect,                         //194 SPELL_AURA_MOD_TARGET_ABSORB_SCHOOL implemented in Unit::CalcAbsorbResist
    &Aura::HandleNoImmediateEffect,                         //195 SPELL_AURA_MOD_TARGET_ABILITY_ABSORB_SCHOOL implemented in Unit::CalcAbsorbResist
    &Aura::HandleNULL,                                      //196 SPELL_AURA_MOD_COOLDOWN - flat mod of spell cooldowns
    &Aura::HandleNoImmediateEffect,                         //197 SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE implemented in Unit::SpellCriticalBonus Unit::GetUnitCriticalChance
    &Aura::HandleUnused,                                    //198 unused (3.0.8a) old SPELL_AURA_MOD_ALL_WEAPON_SKILLS
    &Aura::HandleNoImmediateEffect,                         //199 SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT  implemented in Unit::MagicSpellHitResult
    &Aura::HandleNoImmediateEffect,                         //200 SPELL_AURA_MOD_XP_PCT implemented in Player::RewardPlayerAndGroupAtKill
    &Aura::HandleAuraAllowFlight,                           //201 SPELL_AURA_FLY                             this aura enable flight mode...
    &Aura::HandleNoImmediateEffect,                         //202 SPELL_AURA_CANNOT_BE_DODGED                implemented in Unit::RollPhysicalOutcomeAgainst
    &Aura::HandleNoImmediateEffect,                         //203 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE  implemented in Unit::CalculateMeleeDamage and Unit::CalculateSpellDamage
    &Aura::HandleNoImmediateEffect,                         //204 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE implemented in Unit::CalculateMeleeDamage and Unit::CalculateSpellDamage
    &Aura::HandleNoImmediateEffect,                         //205 SPELL_AURA_MOD_SCHOOL_CRIT_DMG_TAKEN
    &Aura::HandleAuraModIncreaseFlightSpeed,                //206 SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED
    &Aura::HandleAuraModIncreaseFlightSpeed,                //207 SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED
    &Aura::HandleAuraModIncreaseFlightSpeed,                //208 SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED
    &Aura::HandleAuraModIncreaseFlightSpeed,                //209 SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS
    &Aura::HandleAuraModIncreaseFlightSpeed,                //210 SPELL_AURA_MOD_VEHICLE_SPEED_ALWAYS
    &Aura::HandleAuraModIncreaseFlightSpeed,                //211 SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK
    &Aura::HandleAuraModRangedAttackPowerOfStatPercent,     //212 SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT
    &Aura::HandleNoImmediateEffect,                         //213 SPELL_AURA_MOD_RAGE_FROM_DAMAGE_DEALT implemented in Player::RewardRage
    &Aura::HandleNULL,                                      //214 Tamed Pet Passive
    &Aura::HandleArenaPreparation,                          //215 SPELL_AURA_ARENA_PREPARATION
    &Aura::HandleModCastingSpeed,                           //216 SPELL_AURA_HASTE_SPELLS
    &Aura::HandleUnused,                                    //217 unused (3.0.8a)
    &Aura::HandleAuraModRangedHaste,                        //218 SPELL_AURA_HASTE_RANGED
    &Aura::HandleModManaRegen,                              //219 SPELL_AURA_MOD_MANA_REGEN_FROM_STAT
    &Aura::HandleModRatingFromStat,                         //220 SPELL_AURA_MOD_RATING_FROM_STAT
    &Aura::HandleNULL,                                      //221 SPELL_AURA_MOD_DETAUNT
    &Aura::HandleUnused,                                    //222 unused (3.0.8a) only for spell 44586 that not used in real spell cast
    &Aura::HandleNoImmediateEffect,                         //223 SPELL_AURA_RAID_PROC_FROM_CHARGE
    &Aura::HandleUnused,                                    //224 unused (3.0.8a)
    &Aura::HandleNoImmediateEffect,                         //225 SPELL_AURA_RAID_PROC_FROM_CHARGE_WITH_VALUE
    &Aura::HandleAuraPeriodicDummy,                         //226 SPELL_AURA_PERIODIC_DUMMY
    &Aura::HandlePeriodicTriggerSpellWithValue,             //227 SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE
    &Aura::HandleNoImmediateEffect,                         //228 SPELL_AURA_DETECT_STEALTH stealth detection
    &Aura::HandleNoImmediateEffect,                         //229 SPELL_AURA_MOD_AOE_DAMAGE_AVOIDANCE
    &Aura::HandleAuraModIncreaseHealth,                     //230 SPELL_AURA_MOD_INCREASE_HEALTH_2
    &Aura::HandleNoImmediateEffect,                         //231 SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE
    &Aura::HandleNoImmediateEffect,                         //232 SPELL_AURA_MECHANIC_DURATION_MOD           implement in Unit::CalculateSpellDuration
    &Aura::HandleUnused,                                    //233 set model id to the one of the creature with id GetMiscValue() - clientside
    &Aura::HandleNoImmediateEffect,                         //234 SPELL_AURA_MECHANIC_DURATION_MOD_NOT_STACK implement in Unit::CalculateSpellDuration
    &Aura::HandleNoImmediateEffect,                         //235 SPELL_AURA_MOD_DISPEL_RESIST               implement in Unit::MagicSpellHitResult
    &Aura::HandleAuraControlVehicle,                        //236 SPELL_AURA_CONTROL_VEHICLE
    &Aura::HandleModSpellDamagePercentFromAttackPower,      //237 SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER  implemented in Unit::SpellBaseDamageBonus
    &Aura::HandleModSpellHealingPercentFromAttackPower,     //238 SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER implemented in Unit::SpellBaseHealingBonus
    &Aura::HandleAuraModScale,                              //239 SPELL_AURA_MOD_SCALE_2 only in Noggenfogger Elixir (16595) before 2.3.0 aura 61
    &Aura::HandleAuraModExpertise,                          //240 SPELL_AURA_MOD_EXPERTISE
    &Aura::HandleForceMoveForward,                          //241 SPELL_AURA_FORCE_MOVE_FORWARD Forces the player to move forward
    &Aura::HandleUnused,                                    //242 SPELL_AURA_MOD_SPELL_DAMAGE_FROM_HEALING - 2 test spells: 44183 and 44182
    &Aura::HandleNULL,                                      //243 faction reaction override spells
    &Aura::HandleComprehendLanguage,                        //244 SPELL_AURA_COMPREHEND_LANGUAGE
    &Aura::HandleNoImmediateEffect,                         //245 SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL
    &Aura::HandleNoImmediateEffect,                         //246 SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL_NOT_STACK implemented in Spell::EffectApplyAura
    &Aura::HandleAuraCloneCaster,                           //247 SPELL_AURA_CLONE_CASTER
    &Aura::HandleNoImmediateEffect,                         //248 SPELL_AURA_MOD_COMBAT_RESULT_CHANCE         implemented in Unit::RollMeleeOutcomeAgainst
    &Aura::HandleAuraConvertRune,                           //249 SPELL_AURA_CONVERT_RUNE
    &Aura::HandleAuraModIncreaseHealth,                     //250 SPELL_AURA_MOD_INCREASE_HEALTH_2
    &Aura::HandleNoImmediateEffect,                         //251 SPELL_AURA_MOD_ENEMY_DODGE
    &Aura::HandleModCombatSpeedPct,                         //252 SPELL_AURA_252 Is there any difference between this and SPELL_AURA_MELEE_SLOW ? maybe not stacking mod?
    &Aura::HandleNoImmediateEffect,                         //253 SPELL_AURA_MOD_BLOCK_CRIT_CHANCE  implemented in Unit::isBlockCritical
    &Aura::HandleAuraModDisarm,                             //254 SPELL_AURA_MOD_DISARM_OFFHAND
    &Aura::HandleNoImmediateEffect,                         //255 SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT    implemented in Unit::SpellDamageBonus
    &Aura::HandleNoReagentUseAura,                          //256 SPELL_AURA_NO_REAGENT_USE Use SpellClassMask for spell select
    &Aura::HandleNULL,                                      //257 SPELL_AURA_MOD_TARGET_RESIST_BY_SPELL_CLASS Use SpellClassMask for spell select
    &Aura::HandleNULL,                                      //258 SPELL_AURA_MOD_SPELL_VISUAL
    &Aura::HandleNoImmediateEffect,                         //259 SPELL_AURA_MOD_HOT_PCT implemented in Unit::SpellHealingBonus
    &Aura::HandleNoImmediateEffect,                         //260 SPELL_AURA_SCREEN_EFFECT (miscvalue = id in ScreenEffect.dbc) not required any code
    &Aura::HandlePhase,                                     //261 SPELL_AURA_PHASE undetactable invisibility?     implemented in Unit::isVisibleForOrDetect
    &Aura::HandleNoImmediateEffect,                         //262 SPELL_AURA_ABILITY_IGNORE_AURASTATE implemented in spell::cancast
    &Aura::HandleAuraAllowOnlyAbility,                      //263 SPELL_AURA_ALLOW_ONLY_ABILITY player can use only abilities set in SpellClassMask
    &Aura::HandleUnused,                                    //264 unused (3.0.8a)
    &Aura::HandleUnused,                                    //265 unused (3.0.8a)
    &Aura::HandleUnused,                                    //266 unused (3.0.8a)
    &Aura::HandleNoImmediateEffect,                         //267 SPELL_AURA_MOD_IMMUNE_AURA_APPLY_SCHOOL         implemented in Unit::IsImmunedToSpellEffect
    &Aura::HandleAuraModAttackPowerOfStatPercent,           //268 SPELL_AURA_MOD_ATTACK_POWER_OF_STAT_PERCENT
    &Aura::HandleNoImmediateEffect,                         //269 SPELL_AURA_MOD_IGNORE_TARGET_RESIST implemented in Unit::CalcAbsorbResist and CalcArmorReducedDamage
    &Aura::HandleNoImmediateEffect,                         //270 SPELL_AURA_MOD_ABILITY_IGNORE_TARGET_RESIST implemented in Unit::CalcAbsorbResist and CalcArmorReducedDamage
    &Aura::HandleNoImmediateEffect,                         //271 SPELL_AURA_MOD_DAMAGE_FROM_CASTER    implemented in Unit::SpellDamageBonus
    &Aura::HandleNULL,                                      //272 unknown
    &Aura::HandleNoImmediateEffect,                         //273 SPELL_AURA_X_RAY (client side implementation)
    &Aura::HandleNoImmediateEffect,                         //274 SPELL_AURA_CONSUME_NO_AMMO implemented in spell::CalculateDamageDoneForAllTargets
    &Aura::HandleNoImmediateEffect,                         //275 SPELL_AURA_MOD_IGNORE_SHAPESHIFT Use SpellClassMask for spell select
    &Aura::HandleNULL,                                      //276 mod damage % mechanic?
    &Aura::HandleNoImmediateEffect,                         //277 SPELL_AURA_MOD_ABILITY_AFFECTED_TARGETS implemented in spell::settargetmap
    &Aura::HandleAuraModDisarm,                             //278 SPELL_AURA_MOD_DISARM_RANGED disarm ranged weapon
    &Aura::HandleAuraInitializeImages,                      //279 SPELL_AURA_INITIALIZE_IMAGES
    &Aura::HandleNoImmediateEffect,                         //280 SPELL_AURA_MOD_TARGET_ARMOR_PCT
    &Aura::HandleNoImmediateEffect,                         //281 SPELL_AURA_MOD_HONOR_GAIN_PCT implemented in Player::RewardHonor
    &Aura::HandleAuraIncreaseBaseHealthPercent,             //282 SPELL_AURA_INCREASE_BASE_HEALTH_PERCENT
    &Aura::HandleNoImmediateEffect,                         //283 SPELL_AURA_MOD_HEALING_RECEIVED       implemented in Unit::SpellHealingBonus
    &Aura::HandleAuraLinked,                                //284 SPELL_AURA_LINKED    51 spells using
    &Aura::HandleAuraModAttackPowerOfArmor,                 //285 SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR  implemented in Player::UpdateAttackPowerAndDamage
    &Aura::HandleNoImmediateEffect,                         //286 SPELL_AURA_ABILITY_PERIODIC_CRIT implemented in AuraEffect::PeriodicTick
    &Aura::HandleNoImmediateEffect,                         //287 SPELL_AURA_DEFLECT_SPELLS             implemented in Unit::MagicSpellHitResult and Unit::MeleeSpellHitResult
    &Aura::HandleNoImmediateEffect,                         //288 SPELL_AURA_IGNORE_HIT_DIRECTION  implemented in Unit::MagicSpellHitResult and Unit::MeleeSpellHitResult Unit::RollMeleeOutcomeAgainst   
    &Aura::HandleNULL,                                      //289 unused
    &Aura::HandleAuraModCritPct,                            //290 SPELL_AURA_MOD_CRIT_PCT
    &Aura::HandleNoImmediateEffect,                         //291 SPELL_AURA_MOD_XP_QUEST_PCT  implemented in Player::RewardQuest
    &Aura::HandleNULL,                                      //292 call stabled pet
    &Aura::HandleNULL,                                      //293 2 test spells
    &Aura::HandleNoImmediateEffect,                          //294 SPELL_AURA_PREVENT_REGENERATE_POWER implemented in Player::Regenerate(Powers power)
    &Aura::HandleNULL,                                      //295 unused
    &Aura::HandleNULL,                                      //296 2 spells
    &Aura::HandleNULL,                                      //297 1 spell (counter spell school?)
    &Aura::HandleNULL,                                      //298 unused
    &Aura::HandleNULL,                                      //299 unused
    &Aura::HandleNULL,                                      //300 3 spells (share damage?)
    &Aura::HandleNULL,                                      //301 5 spells
    &Aura::HandleNULL,                                      //302 unused
    &Aura::HandleNULL,                                      //303 17 spells
    &Aura::HandleNULL,                                      //304 2 spells (alcohol effect?)
    &Aura::HandleAuraModIncreaseSpeed,                      //305 SPELL_AURA_MOD_MINIMUM_SPEED
    &Aura::HandleNULL                                       //306 1 spell
};
#undef Aura

Aura::Aura(SpellEntry const* spellproto, uint32 effMask, Unit *target, WorldObject *source, Unit *caster, int32 *currentBasePoints, Item* castItem) :
    m_spellProto(spellproto),
    m_target(target), m_sourceGuid(source->GetGUID()), m_casterGuid(caster->GetGUID()), m_castItemGuid(castItem ? castItem->GetGUID() : 0),
    m_applyTime(time(NULL)),
    m_timeCla(0), m_removeMode(AURA_REMOVE_BY_DEFAULT), m_AuraDRGroup(DIMINISHING_NONE),
    m_auraSlot(MAX_AURAS), m_auraLevel(1), m_procCharges(0), m_stackAmount(1), m_isRemoved(false)
{
    assert(target);
    assert(spellproto && spellproto == sSpellStore.LookupEntry( spellproto->Id ) && "`info` must be pointer to sSpellStore element");

    m_auraFlags = effMask;

    if(m_spellProto->manaPerSecond || m_spellProto->manaPerSecondPerLevel)
        m_timeCla = 1000;

    m_isPassive = IsPassiveSpell(GetId());

    m_isSingleTargetAura = IsSingleTargetSpell(m_spellProto);

    //damage        = caster->CalculateSpellDamage(m_spellProto,m_effIndex,m_currentBasePoints,target);
    m_maxduration = caster->CalcSpellDuration(m_spellProto);

    if(m_maxduration == -1 || m_isPassive && m_spellProto->DurationIndex == 0)
        m_permanent = true;
    else
        m_permanent = false;

    Player* modOwner = caster->GetSpellModOwner();

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

    m_isRemovedOnShapeLost = (caster == target &&
                              m_spellProto->Stances &&
                            !(m_spellProto->AttributesEx2 & SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT) &&
                            !(m_spellProto->Attributes & SPELL_ATTR_NOT_SHAPESHIFT));

    for (uint8 i=0 ; i<MAX_SPELL_EFFECTS; ++i)
    {
        if (m_auraFlags & (uint8(1) << i))
        {
            if(!(m_partAuras[i] = CreateAuraEffect(this, i, currentBasePoints ? currentBasePoints + i : NULL)))
                m_auraFlags &= uint8(~(1<< i)); // correct flags if aura couldn't be created
        }
        else
        {
            m_partAuras[i] = NULL;
        }
    }

    // Aura is positive when it is casted by friend and at least one aura is positive
    // or when it is casted by enemy and at least one aura is negative
    bool swap = false;
    if (caster == target) // caster == target - 1 negative effect is enough for aura to be negative
        m_positive = false;
    else
        m_positive = !caster->IsHostileTo(m_target);

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if((1<<i & GetEffectMask()) && m_positive == IsPositiveEffect(GetId(), i))
        {
            swap = true;
            break;
        }
    }
    if (!swap)
        m_positive = !m_positive;
}

Aura::~Aura()
{
    // free part auras memory
    for (uint8 i=0 ; i<MAX_SPELL_EFFECTS; ++i)
        if (m_partAuras[i])
            delete m_partAuras[i];
}

AuraEffect::AuraEffect(Aura *parentAura, uint8 effIndex, int32 *currentBasePoints) :
m_parentAura(parentAura), m_spellmod(NULL), m_periodicTimer(0), m_isPeriodic(false), m_isAreaAura(false), m_isPersistent(false),
m_target(parentAura->GetTarget()), m_tickNumber(0)
, m_spellProto(parentAura->GetSpellProto()), m_effIndex(effIndex), m_auraName(AuraType(m_spellProto->EffectApplyAuraName[m_effIndex]))
{
    assert(m_auraName < TOTAL_AURAS);

    if(currentBasePoints)
        m_currentBasePoints = *currentBasePoints;
    else
        m_currentBasePoints = m_spellProto->EffectBasePoints[m_effIndex];

    Unit *caster = GetParentAura()->GetCaster();
    if(caster)
        m_amount = caster->CalculateSpellDamage(m_spellProto, m_effIndex, m_currentBasePoints, m_target);
    else
        m_amount = m_currentBasePoints + m_spellProto->EffectBaseDice[m_effIndex];

    if (int32 amount = CalculateCrowdControlAuraAmount(caster))
        m_amount = amount;

    if(!m_amount && caster)
        if(uint64 itemGUID = GetParentAura()->GetCastItemGUID())
            if(Player *playerCaster = dynamic_cast<Player*>(caster))
                if(Item *castItem = playerCaster->GetItemByGuid(itemGUID))
    if (castItem->GetItemSuffixFactor())
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

    m_isApplied = false;
}

AreaAuraEffect::AreaAuraEffect(Aura * parentAura, uint32 effIndex, int32 *currentBasePoints)
: AuraEffect(parentAura, effIndex, currentBasePoints)
{
    m_removeTime = FRIENDLY_AA_REMOVE_TIME;
    m_isAreaAura = true;

    if (m_spellProto->Effect[effIndex] == SPELL_EFFECT_APPLY_AREA_AURA_ENEMY)
        m_radius = GetSpellRadiusForHostile(sSpellRadiusStore.LookupEntry(GetSpellProto()->EffectRadiusIndex[m_effIndex]));
    else
        m_radius = GetSpellRadiusForFriend(sSpellRadiusStore.LookupEntry(GetSpellProto()->EffectRadiusIndex[m_effIndex]));

    Unit *source = GetSource();
    assert(source);
    if(Player* modOwner = source->GetSpellModOwner()) // source or caster? should be the same
        modOwner->ApplySpellMod(GetId(), SPELLMOD_RADIUS, m_radius);

    switch(m_spellProto->Effect[effIndex])
    {
        case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
            m_areaAuraType = AREA_AURA_PARTY;
            if(m_target->GetTypeId() == TYPEID_UNIT && ((Creature*)m_target)->isTotem())
                *const_cast<AuraType*>(&m_auraName) = SPELL_AURA_NONE;
            break;
        case SPELL_EFFECT_APPLY_AREA_AURA_RAID:
            m_areaAuraType = AREA_AURA_RAID;
            if(m_target->GetTypeId() == TYPEID_UNIT && ((Creature*)m_target)->isTotem())
                *const_cast<AuraType*>(&m_auraName) = SPELL_AURA_NONE;
            break;
        case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
            m_areaAuraType = AREA_AURA_FRIEND;
            break;
        case SPELL_EFFECT_APPLY_AREA_AURA_ENEMY:
            m_areaAuraType = AREA_AURA_ENEMY;
            if(m_target == source)
                *const_cast<AuraType*>(&m_auraName) = SPELL_AURA_NONE;    // Do not do any effect on self
            break;
        case SPELL_EFFECT_APPLY_AREA_AURA_PET:
            m_areaAuraType = AREA_AURA_PET;
            break;
        case SPELL_EFFECT_APPLY_AREA_AURA_OWNER:
            m_areaAuraType = AREA_AURA_OWNER;
            if(m_target == source)
                *const_cast<AuraType*>(&m_auraName) = SPELL_AURA_NONE;
            break;
        default:
            sLog.outError("Wrong spell effect in AreaAura constructor");
            ASSERT(false);
            break;
    }
}

PersistentAreaAuraEffect::PersistentAreaAuraEffect(Aura * parentAura, uint32 effIndex, int32 *currentBasePoints)
: AuraEffect(parentAura, effIndex, currentBasePoints)
{
    m_isPersistent = true;
}

DynamicObject *PersistentAreaAuraEffect::GetSource() const
{
    uint64 guid = GetParentAura()->GetSourceGUID();
    if(IS_DYNAMICOBJECT_GUID(guid))
        return ObjectAccessor::GetObjectInWorld(guid, (DynamicObject*)NULL);
    return NULL;
}

AuraEffect* CreateAuraEffect(Aura * parentAura, uint32 effIndex, int32 *currentBasePoints)
{
    // TODO: source should belong to aura, but not areaeffect. multiple areaaura/persistent aura should use one source
    assert(parentAura);
    uint64 sourceGuid = parentAura->GetSourceGUID();
    //assert(source);
    if (IsAreaAuraEffect(parentAura->GetSpellProto()->Effect[effIndex]))
    {
        //assert(source->isType(TYPEMASK_UNIT));
        assert(IS_UNIT_GUID(sourceGuid));
        if(!parentAura->GetUnitSource())
        {
            // TODO: there is a crash here when a new aura is added by source aura update, confirmed
            sLog.outCrash("CreateAuraEffect: cannot find source " I64FMT " in world for spell %u", sourceGuid, parentAura->GetId());
            return NULL;
        }
        return new AreaAuraEffect(parentAura, effIndex, currentBasePoints);
    }
    else if (parentAura->GetSpellProto()->Effect[effIndex] == SPELL_EFFECT_APPLY_AURA)
        return new AuraEffect(parentAura, effIndex, currentBasePoints);
    else if (parentAura->GetSpellProto()->Effect[effIndex] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
    {
        //assert(source->isType(TYPEMASK_DYNAMICOBJECT));
        // TODO: creature addon or save? may add persistent AA without correct source
        if(IS_DYNAMICOBJECT_GUID(sourceGuid))
            return new PersistentAreaAuraEffect(parentAura, effIndex, currentBasePoints);
    }
    return NULL;
}

Unit* Aura::GetCaster() const
{
    if(m_casterGuid == m_target->GetGUID())
        return m_target;

    //return ObjectAccessor::GetUnit(*m_target,m_casterGuid);
    //must return caster even if it's in another grid/map
    return ObjectAccessor::GetObjectInWorld(m_casterGuid, (Unit*)NULL);
}

Unit* Aura::GetUnitSource() const
{
    if(m_sourceGuid == m_target->GetGUID())
        return m_target;

    return ObjectAccessor::GetObjectInWorld(m_sourceGuid, (Unit*)NULL);
}

void Aura::Update(uint32 diff)
{
    // TODO: store pointer to caster in aura class for update/mod handling code

    if (m_duration > 0)
    {
        m_duration -= diff;
        if (m_duration < 0)
            m_duration = 0;

        // all spells with manaPerSecond/manaPerSecondPerLevel have aura in effect 0
        if(m_timeCla)
        {
            if(m_timeCla > diff)
                m_timeCla -= diff;
            else if(Unit* caster = GetCaster())
            {
                if(int32 manaPerSecond = m_spellProto->manaPerSecond + m_spellProto->manaPerSecondPerLevel * caster->getLevel())
                {
                    m_timeCla += 1000 - diff;

                    Powers powertype = Powers(m_spellProto->powerType);
                    if(powertype == POWER_HEALTH)
                    {
                        if (caster->GetHealth() > manaPerSecond)
                            caster->ModifyHealth(-manaPerSecond);
                        else
                        {
                            m_target->RemoveAura(this);
                            return;
                        }
                    }
                    else
                    {
                        if (caster->GetPower(powertype) >= manaPerSecond)
                            caster->ModifyPower(powertype, -manaPerSecond);
                        else
                        {
                            m_target->RemoveAura(this);
                            return;
                        }
                    }
                }
            }
        }
    }

    // Apply charged spellmods for channeled auras
    // used for example when triggered spell of spell:10 is modded
    Spell *modSpell = NULL;
    Player *modOwner = NULL;
    if(IS_PLAYER_GUID(GetCasterGUID()) && (modOwner = (Player*)GetCaster())
        && (modSpell = modOwner->FindCurrentSpellBySpellId(GetId())))
        modOwner->SetSpellModTakingSpell(modSpell, true);

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (m_partAuras[i])
            m_partAuras[i]->Update(diff);

    if (modOwner)
        modOwner->SetSpellModTakingSpell(modSpell, false);
}

void AuraEffect::Update(uint32 diff)
{
    if (m_isPeriodic && (GetParentAura()->GetAuraDuration() >=0 || GetParentAura()->IsPassive() || GetParentAura()->IsPermanent()))
    {
        if(m_periodicTimer > diff)
            m_periodicTimer -= diff;
        else // tick also at m_periodicTimer==0 to prevent lost last tick in case max m_duration == (max m_periodicTimer)*N
        {
            ++m_tickNumber;

            // update before applying (aura can be removed in TriggerSpell or PeriodicTick calls)
            m_periodicTimer += m_amplitude - diff;

            if(!m_target->hasUnitState(UNIT_STAT_ISOLATED))
                PeriodicTick();
        }
    }
}

void AreaAuraEffect::Update(uint32 diff)
{
    // update for the source of the aura
    if(GetParentAura()->GetSourceGUID() == m_target->GetGUID())
    {
        Unit *source = m_target;
        Unit *caster = GetCaster();
        if (!caster)
        {
            m_target->RemoveAura(GetParentAura());
            return;
        }

        if( !source->hasUnitState(UNIT_STAT_ISOLATED) )
        {
            std::list<Unit *> targets;

            switch(m_areaAuraType)
            {
                case AREA_AURA_PARTY:
                    source->GetPartyMemberInDist(targets, m_radius);
                    break;
                case AREA_AURA_RAID:
                    source->GetRaidMember(targets, m_radius);
                    break;
                case AREA_AURA_FRIEND:
                {
                    Trinity::AnyFriendlyUnitInObjectRangeCheck u_check(source, caster, m_radius);
                    Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(source, targets, u_check);
                    source->VisitNearbyObject(m_radius, searcher);
                    break;
                }
                case AREA_AURA_ENEMY:
                {
                    Trinity::AnyAoETargetUnitInObjectRangeCheck u_check(source, caster, m_radius); // No GetCharmer in searcher
                    Trinity::UnitListSearcher<Trinity::AnyAoETargetUnitInObjectRangeCheck> searcher(source, targets, u_check);
                    source->VisitNearbyObject(m_radius, searcher);
                    break;
                }
                case AREA_AURA_OWNER:
                case AREA_AURA_PET:
                {
                    if(Unit *owner = caster->GetCharmerOrOwner())
                        if (caster->IsWithinDistInMap(owner, m_radius))
                            targets.push_back(owner);
                    break;
                }
            }

            for (std::list<Unit*>::iterator tIter = targets.begin(); tIter != targets.end(); tIter++)
            {
                if(Aura *aur = (*tIter)->GetAura(GetId(), GetCasterGUID()))
                {
                    if(aur->HasEffect(GetEffIndex()))
                        continue;
                }
                else
                {
                    bool skip = false;
                    for (Unit::AuraMap::iterator iter = (*tIter)->GetAuras().begin(); iter != (*tIter)->GetAuras().end(); ++iter)
                    {
                        if(!spellmgr.CanAurasStack(GetSpellProto(), iter->second->GetSpellProto(), iter->second->GetCasterGUID() == GetCasterGUID()))
                        {
                            skip = true;
                            break;
                        }
                    }
                    if(skip)
                        continue;
                }

                // Select lower rank of aura if needed
                if(SpellEntry const *actualSpellInfo = spellmgr.SelectAuraRankForPlayerLevel(GetSpellProto(), (*tIter)->getLevel()))
                {
                    int32 newBp = m_currentBasePoints;
                    // Check if basepoints can be safely reduced
                    if (newBp == m_spellProto->EffectBasePoints[m_effIndex])
                        newBp = actualSpellInfo->EffectBasePoints[m_effIndex];
                    (*tIter)->AddAuraEffect(actualSpellInfo, GetEffIndex(), source, caster, &newBp);

                    if(m_areaAuraType == AREA_AURA_ENEMY)
                        caster->CombatStart(*tIter);
                }
            }
        }
        AuraEffect::Update(diff);
    }
    else                                                    // aura at non-caster
    {
        // WARNING: the aura may get deleted during the update
        // DO NOT access its members after update!
        AuraEffect::Update(diff);

        // Speedup - no need to do more checks
        if (GetParentAura()->IsRemoved())
            return;

        // Caster may be deleted due to update
        Unit *caster = GetCaster();
        Unit *source = GetSource();

        // remove aura if out-of-range from caster (after teleport for example)
        // or caster is isolated or caster no longer has the aura
        // or caster is (no longer) friendly
        bool needFriendly = (m_areaAuraType == AREA_AURA_ENEMY ? false : true);
        if( !source || !caster ||
            source->hasUnitState(UNIT_STAT_ISOLATED) || !source->HasAuraEffect(GetId(), m_effIndex) ||
            caster->IsFriendlyTo(m_target) != needFriendly
           )
        {
            m_target->RemoveAura(GetParentAura());
        }
        else if (!source->IsWithinDistInMap(m_target, m_radius))
        {
            if (needFriendly && source->isMoving())
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
                if(!m_target->IsInPartyWith(caster))
                    m_target->RemoveAura(GetParentAura());
            }
            else if( m_areaAuraType == AREA_AURA_RAID)
            {
                if(!m_target->IsInRaidWith(caster))
                    m_target->RemoveAura(GetParentAura());
            }
            else if( m_areaAuraType == AREA_AURA_PET || m_areaAuraType == AREA_AURA_OWNER )
            {
                if( m_target->GetGUID() != caster->GetCharmerOrOwnerGUID() )
                    m_target->RemoveAura(GetParentAura());
            }
        }
    }
}

void PersistentAreaAuraEffect::Update(uint32 diff)
{
    /*
    if(Unit *caster = GetParentAura()->GetCaster())
    {
        if(DynamicObject *dynObj = caster->GetDynObject(GetId(), GetEffIndex()))
        {
            if(m_target->IsWithinDistInMap(dynObj, dynObj->GetRadius()))
            {
                AuraEffect::Update(diff);
                return;
            }
        }
    }
    */
    if(DynamicObject *dynObj = GetSource())
    {
        if(m_target->IsWithinDistInMap(dynObj, dynObj->GetRadius()))
        {
            AuraEffect::Update(diff);
            return;
        }
    }

    // remove the aura if its caster or the dynamic object causing it was removed
    // or if the target moves too far from the dynamic object
    m_target->RemoveAura(GetParentAura());
}

void AuraEffect::ApplyModifier(bool apply, bool Real, bool changeAmount)
{
    if (GetParentAura()->IsRemoved())
        return;

    if (apply)
        HandleAuraEffectSpecificMods(true, Real, changeAmount);

    (*this.*AuraHandler [m_auraName])(apply,Real, changeAmount);

    if (!apply)
        HandleAuraEffectSpecificMods(false, Real, changeAmount);
}

void AuraEffect::RecalculateAmount(bool applied)
{
    Unit *caster = GetParentAura()->GetCaster();
    int32 amount = GetParentAura()->GetStackAmount() * (caster ? (caster->CalculateSpellDamage(m_spellProto, GetEffIndex(), GetBasePoints(), NULL)) : (m_currentBasePoints + m_spellProto->EffectBaseDice[m_effIndex]));
    // Reapply if amount change
    if (amount!=GetAmount())
    {
        // Auras which are applying spellmod should have removed spellmods for real
        if (applied)
            ApplyModifier(false,false,true);
        SetAmount(amount);
        if (applied)
            ApplyModifier(true,false,true);
    }
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

void AuraEffect::HandleSchoolAbsorb(bool apply, bool Real, bool changeAmount)
{
    if(!Real)
        return;

    Unit* caster = GetCaster();
    if(!caster)
        return;

    if (apply)
    {
        // prevent double apply bonuses
        if (m_target->GetTypeId() != TYPEID_PLAYER || !((Player*)m_target)->GetSession()->PlayerLoading())
        {
            float DoneActualBenefit = 0.0f;
            switch (m_spellProto->SpellFamilyName)
            {
                case SPELLFAMILY_PRIEST:
                    // Power Word: Shield
                    if (m_spellProto->SpellFamilyFlags[0] & 0x00000001)
                        //+80.68% from +spell bonus
                        DoneActualBenefit = caster->SpellBaseHealingBonus(GetSpellSchoolMask(m_spellProto)) * 0.8068f;
                    break;
                case SPELLFAMILY_MAGE:
                    // Frost Ward, Fire Ward
                    if (m_spellProto->SpellFamilyFlags[0] & 0x00000108)
                        //+10% from +spell bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonus(GetSpellSchoolMask(m_spellProto)) * 0.1f;
                    // Ice Barrier
                    else if (m_spellProto->SpellFamilyFlags[1] & 0x00000001 && m_spellProto->SpellIconID == 32)
                        //+80.67% from +spell bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonus(GetSpellSchoolMask(m_spellProto)) * 0.8067f;
                    break;
                case SPELLFAMILY_WARLOCK:
                    // Shadow Ward
                    if (m_spellProto->SpellIconID == 207)
                        //+30% from +spell bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonus(GetSpellSchoolMask(m_spellProto)) * 0.30f;
                    break;
                case SPELLFAMILY_PALADIN:
                    // Sacred Shield
                    // (check not strictly needed, only Sacred Shield has SPELL_AURA_SCHOOL_ABSORB in SPELLFAMILY_PALADIN at this time)
                    if (m_spellProto->SpellFamilyFlags[1] & 0x00080000)
                    {
                        // +75% from spell power
                        DoneActualBenefit = caster->SpellBaseHealingBonus(GetSpellSchoolMask(m_spellProto)) * 0.75f;
                    }
                    break;
                default:
                    break;
            }

            DoneActualBenefit *= caster->CalculateLevelPenalty(GetSpellProto());

            m_amount += (int32)DoneActualBenefit;
        }
    }
    else
    {
        if (caster &&
            // Power Word: Shield
            m_spellProto->SpellFamilyName == SPELLFAMILY_PRIEST && m_spellProto->Mechanic == MECHANIC_SHIELD &&
            (m_spellProto->SpellFamilyFlags & UI64LIT(0x0000000000000001)) &&
            // completely absorbed or dispelled
            ((m_parentAura->GetRemoveMode() == AURA_REMOVE_BY_DEFAULT && m_amount) || m_parentAura->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE))
        {
            Unit::AuraEffectList const& vDummyAuras = caster->GetAurasByType(SPELL_AURA_DUMMY);
            for(Unit::AuraEffectList::const_iterator itr = vDummyAuras.begin(); itr != vDummyAuras.end(); ++itr)
            {
                SpellEntry const* vSpell = (*itr)->GetSpellProto();

                // Rapture (main spell)
                if(vSpell->SpellFamilyName == SPELLFAMILY_PRIEST && vSpell->SpellIconID == 2894 && vSpell->Effect[1])
                {
                    switch((*itr)->m_effIndex)
                    {
                        case 0:
                        {
                            // energize caster
                            int32 manapct1000 = 5 * ((*itr)->m_amount + spellmgr.GetSpellRank(vSpell->Id));
                            int32 basepoints0 = caster->GetMaxPower(POWER_MANA) * manapct1000 / 1000;
                            caster->CastCustomSpell(caster, 47755, &basepoints0, NULL, NULL, true);
                            break;
                        }
                        case 1:
                        {
                            // energize target
                            if (!roll_chance_i((*itr)->m_amount) || caster->HasAura(63853))
                                break;

                            switch(m_target->getPowerType())
                            {
                            case POWER_RUNIC_POWER:
                                m_target->CastSpell(m_target, 63652, true, NULL, NULL, GetCasterGUID());
                                break;
                            case POWER_RAGE:
                                m_target->CastSpell(m_target, 63653, true, NULL, NULL, GetCasterGUID());
                                break;
                            case POWER_MANA:
                                {
                                    int32 basepoints0 = m_target->GetMaxPower(POWER_MANA) * 2 / 100;
                                    m_target->CastCustomSpell(m_target, 63654, &basepoints0, NULL, NULL, true);
                                    break;
                                }
                            case POWER_ENERGY:
                                m_target->CastSpell(m_target, 63655, true, NULL, NULL, GetCasterGUID());
                                break;
                            default:
                                break;
                            }

                            //cooldwon aura
                            caster->CastSpell(caster, 63853, true);
                            break;
                        }
                        default:
                            sLog.outError("Changes in R-dummy spell???: effect 3");
                            break;
                    }
                }
            }
        }
    }
}

void Aura::ApplyAllModifiers(bool apply, bool Real)
{
    for (uint8 i = 0; i<MAX_SPELL_EFFECTS; ++i)
        if (m_partAuras[i])
            m_partAuras[i]->ApplyModifier(apply, Real);
}

void Aura::HandleAuraSpecificMods(bool apply)
{
    //**************************************************************************************
    // Function called after applying all mods from aura or after removing all mods from it
    //**************************************************************************************

    //********************
    // MODS AT AURA APPLY
    //********************
    if (apply)
    {
        // Update auras for specific phase
        if(IsAuraType(SPELL_AURA_PHASE))
        {
            SpellAreaForAreaMapBounds saBounds = spellmgr.GetSpellAreaForAuraMapBounds(GetId());
            if(saBounds.first != saBounds.second)
            {
                uint32 zone, area;
                m_target->GetZoneAndAreaId(zone,area);

                for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
                {
                    // some auras remove at aura remove
                    if(!itr->second->IsFitToRequirements((Player*)m_target,zone,area))
                        m_target->RemoveAurasDueToSpell(itr->second->spellId);
                    // some auras applied at aura apply
                    else if(itr->second->autocast)
                    {
                        if( !m_target->HasAura(itr->second->spellId) )
                            m_target->CastSpell(m_target,itr->second->spellId,true);
                    }
                }
            }
        }

        if (m_spellProto->SpellFamilyName == SPELLFAMILY_MAGE)
        {
            if (m_spellProto->SpellFamilyFlags[1] & 0x00000002 && m_spellProto->SpellFamilyFlags[2] & 0x00000008)
            {
                // Arcane Potency
                if (Unit * caster = GetCaster())
                {
                    if (AuraEffect* aureff = caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_MAGE, 2120, 0))
                    {
                        if (roll_chance_i(aureff->GetAmount()))
                        {
                            uint32 spell_id = 0;

                            switch (aureff->GetId())
                            {
                                case 31571: spell_id = 57529; break;
                                case 31572: spell_id = 57531; break;
                                default: return;
                            }
                            if(spell_id)
                                caster->CastSpell(caster,spell_id,true);
                        }
                    }
                }
            }
            else if (m_spellProto->SpellFamilyFlags[0] & 0x00000001 && m_spellProto->SpellFamilyFlags[2] & 0x00000008)
            {
                // Glyph of Fireball
                if (Unit * caster = GetCaster())
                    if (caster->HasAura(56368))
                        SetAuraDuration(0);
            }
            else if (m_spellProto->SpellFamilyFlags[0] & 0x00000020 && m_spellProto->SpellVisual[0] == 13)
            {
                // Glyph of Frostbolt
                if (Unit * caster = GetCaster())
                    if (caster->HasAura(56370))
                        SetAuraDuration(0);
            }
            // Todo: This should be moved to similar function in spell::hit
            else if (m_spellProto->SpellFamilyFlags[0] & 0x01000000)
            {
                Unit * caster = GetCaster();
                if (!caster)
                    return;

                // Polymorph Sound - Sheep && Penguin
                if (m_spellProto->SpellIconID == 82 && m_spellProto->SpellVisual[0] == 12978)
                {
                    // Glyph of the Penguin
                    if (caster->HasAura(52648))
                        caster->CastSpell(m_target,61635,true);
                    else
                        caster->CastSpell(m_target,61634,true);
                }
            }
        }
        else if (GetSpellProto()->SpellFamilyName == SPELLFAMILY_PRIEST)
        {
            // Devouring Plague
            if (GetSpellProto()->SpellFamilyFlags[0] & 0x02000000 && GetPartAura(0))
            {
                Unit * caster = GetCaster();
                if (!caster)
                    return;

                // Improved Devouring Plague
                if (AuraEffect const * aurEff = caster->GetDummyAura(SPELLFAMILY_PRIEST, 3790, 1))
                {
                    int32 basepoints0 = aurEff->GetAmount() * GetPartAura(0)->GetTotalTicks() * GetPartAura(0)->GetAmount() / 100;
                    caster->CastCustomSpell(m_target, 63675, &basepoints0, NULL, NULL, true, NULL, GetPartAura(0));
                }
            }
            // Renew
            else if (GetSpellProto()->SpellFamilyFlags[0] & 0x00000040 && GetPartAura(0))
            {
                Unit * caster = GetCaster();
                if (!caster)
                    return;

                // Empowered Renew
                if (AuraEffect const * aurEff = caster->GetDummyAura(SPELLFAMILY_PRIEST, 3021, 1))
                {
                    int32 basepoints0 = aurEff->GetAmount() * GetPartAura(0)->GetTotalTicks() * caster->SpellHealingBonus(m_target, GetSpellProto(), GetPartAura(0)->GetAmount(), HEAL) / 100;
                    caster->CastCustomSpell(m_target, 63544, &basepoints0, NULL, NULL, true, NULL, GetPartAura(0));
                }
            }
        }
        else if (GetSpellProto()->SpellFamilyName == SPELLFAMILY_ROGUE)
        {
            // Sprint (skip non player casted spells by category)
            if(GetSpellProto()->SpellFamilyFlags[0] & 0x40 && GetSpellProto()->Category == 44)
                // in official maybe there is only one icon?
                if(m_target->HasAura(58039)) // Glyph of Blurred Speed
                    m_target->CastSpell(m_target, 61922, true); // Sprint (waterwalk)
        }
        else if (GetSpellProto()->SpellFamilyName == SPELLFAMILY_DEATHKNIGHT)
        {
            // Frost Fever and Blood Plague
            if(GetSpellProto()->SpellFamilyFlags[2] & 0x2)
            {
                // Can't proc on self
                if (GetCasterGUID() == m_target->GetGUID())
                    return;
                Unit * caster = GetCaster();
                if (!caster)
                    return;

                AuraEffect * aurEff = NULL;
                // Ebon Plaguebringer / Crypt Fever
                Unit::AuraEffectList const& TalentAuras = caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                for (Unit::AuraEffectList::const_iterator itr = TalentAuras.begin(); itr != TalentAuras.end(); ++itr)
                {
                    if ((*itr)->GetMiscValue() == 7282)
                    {
                        aurEff = *itr;
                        // Ebon Plaguebringer - end search if found
                        if ((*itr)->GetSpellProto()->SpellIconID == 1766)
                            break;
                    }
                }
                if (aurEff)
                {
                    uint32 spellId = 0;
                    switch (aurEff->GetId())
                    {
                        // Ebon Plague
                        case 51161: spellId = 51735; break;
                        case 51160: spellId = 51734; break;
                        case 51099: spellId = 51726; break;
                        // Crypt Fever
                        case 49632: spellId = 50510; break;
                        case 49631: spellId = 50509; break;
                        case 49032: spellId = 50508; break;
                        default:
                            sLog.outError("Unknown rank of Crypt Fever/Ebon Plague %d", aurEff->GetId());
                    }
                    caster->CastSpell(m_target, spellId, true, 0, GetPartAura(0));
                }
            }
        }
        else
        {
            switch(GetId())
            {
                case 32474: // Buffeting Winds of Susurrus
                    if(m_target->GetTypeId() == TYPEID_PLAYER)
                        ((Player*)m_target)->ActivateTaxiPathTo(506, GetId());
                    break;
                case 33572: // Gronn Lord's Grasp, becomes stoned
                    if(GetStackAmount() >= 5 && !m_target->HasAura(33652))
                        m_target->CastSpell(m_target, 33652, true);
                    break;
                case 48020: // Demonic Circle
                    if(m_target->GetTypeId() == TYPEID_PLAYER)
                        if(GameObject* obj = m_target->GetGameObject(48018))
                            ((Player*)m_target)->TeleportTo(obj->GetMapId(),obj->GetPositionX(),obj->GetPositionY(),obj->GetPositionZ(),obj->GetOrientation());
                    break;
                case 60970: // Heroic Fury (remove Intercept cooldown)
                    if(m_target->GetTypeId() == TYPEID_PLAYER)
                        ((Player*)m_target)->RemoveSpellCooldown(20252, true);
                    break;
            }
        }
    }

    //*******************************
    // MODS AT AURA APPLY AND REMOVE
    //*******************************

    // Aura Mastery Triggered Spell Handler
    // If apply Concentration Aura -> trigger -> apply Aura Mastery Immunity
    // If remove Concentration Aura -> trigger -> remove Aura Mastery Immunity
    // If remove Aura Mastery -> trigger -> remove Aura Mastery Immunity
    if (m_spellProto->Id == 19746 || m_spellProto->Id == 31821)
    {
        if (GetCasterGUID() != m_target->GetGUID())
            return;
        if (apply)
        {
            if ((m_spellProto->Id == 31821 && m_target->HasAura(19746, GetCasterGUID())) || (m_spellProto->Id == 19746 && m_target->HasAura(31821)))
            {
                m_target->CastSpell(m_target,64364,true);
                return;
            }
        }
        else
        {
            m_target->RemoveAurasDueToSpell(64364, GetCasterGUID());
            return;
        }
    }

    // Bestial Wrath
    if (GetSpellProto()->Id == 19574)
    {
        // The Beast Within cast on owner if talent present
        if ( Unit* owner = m_target->GetOwner() )
        {
            // Search talent
            if (owner->HasAura(34692))
            {
                if (apply)
                    owner->CastSpell(owner, 34471, true, 0, GetPartAura(0));
                else
                    owner->RemoveAurasDueToSpell(34471);
            }
        }
    }

    if (GetSpellSpecific(m_spellProto->Id) == SPELL_PRESENCE)
    {
        AuraEffect *bloodPresenceAura=0;  // healing by damage done
        AuraEffect *frostPresenceAura=0;  // increased health
        AuraEffect *unholyPresenceAura=0; // increased movement speed, faster rune recovery

        // Improved Presences
        Unit::AuraEffectList const& vDummyAuras = m_target->GetAurasByType(SPELL_AURA_DUMMY);
        for (Unit::AuraEffectList::const_iterator itr = vDummyAuras.begin(); itr != vDummyAuras.end(); ++itr)
        {
            switch((*itr)->GetId())
            {
                // Improved Blood Presence
                case 50365:
                case 50371:
                {
                    bloodPresenceAura = (*itr);
                    break;
                }
                // Improved Frost Presence
                case 50384:
                case 50385:
                {
                    frostPresenceAura = (*itr);
                    break;
                }
                // Improved Unholy Presence
                case 50391:
                case 50392:
                {
                    unholyPresenceAura = (*itr);
                    break;
                }
            }
        }

        uint32 presence=GetId();
        if (apply)
        {
            // Blood Presence bonus
            if (presence == SPELL_ID_BLOOD_PRESENCE)
                m_target->CastSpell(m_target,63611,true);
            else if (bloodPresenceAura)
            {
                int32 basePoints1=bloodPresenceAura->GetAmount();
                m_target->CastCustomSpell(m_target,63611,NULL,&basePoints1,NULL,true,0,bloodPresenceAura);
            }
            // Frost Presence bonus
            if (presence == SPELL_ID_FROST_PRESENCE)
                m_target->CastSpell(m_target,61261,true);
            else if (frostPresenceAura)
            {
                int32 basePoints0=frostPresenceAura->GetAmount();
                m_target->CastCustomSpell(m_target,61261,&basePoints0,NULL,NULL,true,0,frostPresenceAura);
            }
            // Unholy Presence bonus
            if (presence == SPELL_ID_UNHOLY_PRESENCE)
            {
                if(unholyPresenceAura)
                {
                    // Not listed as any effect, only base points set
                    int32 basePoints0 = unholyPresenceAura->GetSpellProto()->EffectBasePoints[1];
                    //m_target->CastCustomSpell(m_target,63622,&basePoints0 ,NULL,NULL,true,0,unholyPresenceAura);
                    m_target->CastCustomSpell(m_target,65095,&basePoints0 ,NULL,NULL,true,0,unholyPresenceAura);
                }
                m_target->CastSpell(m_target,49772, true);
            }
            else if (unholyPresenceAura)
            {
                int32 basePoints0=unholyPresenceAura->GetAmount();
                m_target->CastCustomSpell(m_target,49772,&basePoints0,NULL,NULL,true,0,unholyPresenceAura);
            }
        }
        else
        {
            // Remove passive auras
            if (presence == SPELL_ID_BLOOD_PRESENCE || bloodPresenceAura)
                m_target->RemoveAurasDueToSpell(63611);
            if (presence == SPELL_ID_FROST_PRESENCE || frostPresenceAura)
                m_target->RemoveAurasDueToSpell(61261);
            if (presence == SPELL_ID_UNHOLY_PRESENCE || unholyPresenceAura)
            {
                if(presence == SPELL_ID_UNHOLY_PRESENCE && unholyPresenceAura)
                {
                    //m_target->RemoveAurasDueToSpell(63622);
                    m_target->RemoveAurasDueToSpell(65095);
                }
                m_target->RemoveAurasDueToSpell(49772);
            }
        }
    }

    //*********************
    // MODS AT AURA REMOVE
    //*********************

    if(!apply)
    {
        // Spell Reflection
        if (m_spellProto->SpellFamilyName == SPELLFAMILY_WARRIOR && m_spellProto->SpellFamilyFlags[1] & 0x2
            && GetRemoveMode() != AURA_REMOVE_BY_DEFAULT)
        {
            if (Unit * caster = GetCaster())
            {
                // Improved Spell Reflection
                if (caster->GetDummyAura(SPELLFAMILY_WARRIOR,1935, 1))
                {
                    // aura remove - remove auras from all party members
                    std::list<Unit*> PartyMembers;
                    m_target->GetPartyMembers(PartyMembers);
                    for (std::list<Unit*>::iterator itr = PartyMembers.begin(); itr!=PartyMembers.end(); ++itr)
                    {
                        if ((*itr)!= m_target)
                            (*itr)->RemoveAurasWithFamily(SPELLFAMILY_WARRIOR, 0, 0x2, 0, GetCasterGUID());
                    }
                }
            }
        }
        // Guardian Spirit
        else if(m_spellProto->Id == 47788)
        {
            if (GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                return;
            Unit *caster = GetCaster();
            if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
                return;

            Player *player = ((Player*)caster);
            // Glyph of Guardian Spirit
            if(AuraEffect * aurEff = player->GetAuraEffect(63231, 0))
            {
                if (!player->HasSpellCooldown(47788))
                    return;

                player->RemoveSpellCooldown(m_spellProto->Id, true);
                player->AddSpellCooldown(m_spellProto->Id, 0, uint32(time(NULL) + aurEff->GetAmount()));

                WorldPacket data(SMSG_SPELL_COOLDOWN, 8+1+4+4);
                data << uint64(player->GetGUID());
                data << uint8(0x0);                                     // flags (0x1, 0x2)
                data << uint32(m_spellProto->Id);
                data << uint32(aurEff->GetAmount()*IN_MILISECONDS);
                player->SendDirectMessage(&data);
            }
        }
        // Invisibility
        else if (m_spellProto->Id == 66)
        {
            if (GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                return;
            m_target->CastSpell(m_target, 32612, true, NULL, GetPartAura(1));
        }
        // Summon Gargoyle
        else if (m_spellProto->Id == 50514)
        {
            m_target->CastSpell(m_target, GetPartAura(0)->GetAmount(), true, NULL, GetPartAura(0));
        }
        // Curse of Doom
        else if (m_spellProto->SpellFamilyName==SPELLFAMILY_WARLOCK && m_spellProto->SpellFamilyFlags[1] & 0x02)
        {
            if (GetRemoveMode()==AURA_REMOVE_BY_DEATH)
            {
                if (Unit * caster = GetCaster())
                {
                    if (caster->GetTypeId() == TYPEID_PLAYER && ((Player*)caster)->isHonorOrXPTarget(m_target))
                        caster->CastSpell(m_target, 18662, true, NULL, GetPartAura(0));
                }
            }
        }
        // Improved Fear
        else if (m_spellProto->SpellFamilyName==SPELLFAMILY_WARLOCK && m_spellProto->SpellFamilyFlags[1] & 0x00000400)
        {
            if (Unit * caster = GetCaster())
            {
                if (caster->GetTypeId() != TYPEID_PLAYER)
                {
                    if (AuraEffect* aureff = caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_WARLOCK, 98, 0))
                    {
                        uint32 spell_id = 0;
                        switch (aureff->GetId())
                        {
                            case 53759: spell_id = 60947; break;
                            case 53754: spell_id = 60946; break;
                            default: return;
                        }
                        if (spell_id)
                            caster->CastSpell(caster,spell_id,true);
                    }
                }
            }
        }
    }
}

void AuraEffect::HandleAuraEffectSpecificMods(bool apply, bool Real, bool changeAmount)
{
    //***********************************************************************
    // Function called before aura effect handler apply or after it's remove
    //***********************************************************************

    if(!Real && !changeAmount)
        return;

    if(apply)
    {
        // prevent double apply bonuses
        if (!m_target->isBeingLoaded())
        if(Unit* caster = GetCaster())
        {
            float DoneActualBenefit = 0.0f;
            switch(m_spellProto->SpellFamilyName)
            {
                case SPELLFAMILY_GENERIC:
                    // Replenishment (0.25% from max)
                    // Infinite Replenishment
                    if (m_spellProto->SpellIconID == 3184 && m_spellProto->SpellVisual[0] == 12495 && GetAuraName() == SPELL_AURA_PERIODIC_ENERGIZE)
                        m_amount = m_target->GetMaxPower(POWER_MANA) * 25 / 10000;
                    break;
                case SPELLFAMILY_MAGE:
                    // Mana Shield
                    if(m_spellProto->SpellFamilyFlags[0] & 0x8000 && m_spellProto->SpellFamilyFlags[2] & 0x8 && GetAuraName() == SPELL_AURA_MANA_SHIELD)
                    {
                        // +80.53% from +spd bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonus(GetSpellSchoolMask(m_spellProto)) * 0.8053f;;
                    }
                    // Ice Barrier
                    else if(m_spellProto->SpellFamilyFlags[1] & 0x1 && m_spellProto->SpellFamilyFlags[2] & 0x8 && GetAuraName() == SPELL_AURA_SCHOOL_ABSORB)
                    {
                        // +80.67% from sp bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonus(GetSpellSchoolMask(m_spellProto)) * 0.8067f;
                    }
                    break;
                case SPELLFAMILY_WARRIOR:
                {
                    // Rend
                    if (m_spellProto->SpellFamilyFlags[0] & 0x20 && GetAuraName() == SPELL_AURA_PERIODIC_DAMAGE)
                    {
                        // $0.2*(($MWB+$mwb)/2+$AP/14*$MWS) bonus per tick
                        float ap = caster->GetTotalAttackPowerValue(BASE_ATTACK);
                        int32 mws = caster->GetAttackTime(BASE_ATTACK);
                        float mwb_min = caster->GetWeaponDamageRange(BASE_ATTACK,MINDAMAGE);
                        float mwb_max = caster->GetWeaponDamageRange(BASE_ATTACK,MAXDAMAGE);
                        m_amount+=int32(((mwb_min+mwb_max)/2+ap*mws/14000)*0.2f);
                        // "If used while your target is above 75% health, Rend does 35% more damage."
                        // as for 3.1.3 only ranks above 9 (wrong tooltip?)
                        if (spellmgr.GetSpellRank(m_spellProto->Id) >= 9)
                        {
                            if (m_target->HasAuraState(AURA_STATE_HEALTH_ABOVE_75_PERCENT, m_spellProto, caster))
                                m_amount += int32(m_amount*0.35);
                        }
                    }
                    break;
                }
                case SPELLFAMILY_WARLOCK:
                    // shadow ward
                    if(m_spellProto->SpellFamilyFlags[2]& 0x40 && GetAuraName() == SPELL_AURA_SCHOOL_ABSORB)
                    {
                        // +30% from sp bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonus(GetSpellSchoolMask(m_spellProto)) * 0.3f;
                    }
                    // Drain Soul - If the target is at or below 25% health, Drain Soul causes four times the normal damage
                    else if (m_spellProto->SpellFamilyFlags[0] & 0x00004000 && GetAuraName() == SPELL_AURA_PERIODIC_DAMAGE)
                    {
                        // if victim is below 25% of hp
                        if (m_target->GetMaxHealth() / 4 > m_target->GetHealth())
                            m_amount *= 4;
                    }
                    break;
                case SPELLFAMILY_PRIEST:
                {
                    // Power Word: Shield
                    if(m_spellProto->SpellFamilyFlags[0] & 0x1 && m_spellProto->SpellFamilyFlags[2] & 0x400 && GetAuraName() == SPELL_AURA_SCHOOL_ABSORB)
                    {
                        //+80.68% from sp bonus
                        DoneActualBenefit = caster->SpellBaseHealingBonus(GetSpellSchoolMask(m_spellProto)) * 0.8068f;
                    }
                    // Borrowed Time
                    else if(m_spellProto->SpellFamilyFlags[0] & 0x1 && GetAuraName() == SPELL_AURA_SCHOOL_ABSORB)
                    {
                        switch(m_spellProto->Id)
                        {
                            case 52795:
                                DoneActualBenefit = caster->SpellBaseHealingBonus(GetSpellSchoolMask(m_spellProto)) * 0.08f; break;
                            case 52797:
                                DoneActualBenefit = caster->SpellBaseHealingBonus(GetSpellSchoolMask(m_spellProto)) * 0.16f; break;
                            case 52798:
                                DoneActualBenefit = caster->SpellBaseHealingBonus(GetSpellSchoolMask(m_spellProto)) * 0.24f; break;
                            case 52799:
                                DoneActualBenefit = caster->SpellBaseHealingBonus(GetSpellSchoolMask(m_spellProto)) * 0.32f; break;
                            case 52800:
                                DoneActualBenefit = caster->SpellBaseHealingBonus(GetSpellSchoolMask(m_spellProto)) * 0.40f; break;
                            default:
                                sLog.outDetail("Unhandled spell '%u' (possibly a new rank of Borrowed Time?) found.",m_spellProto->Id); break;
                        }
                    }
                    break;
                }
                case SPELLFAMILY_DRUID:
                {
                    // Rip
                    if (m_spellProto->SpellFamilyFlags[0] & 0x00800000 && GetAuraName() == SPELL_AURA_PERIODIC_DAMAGE)
                    {
                        // 0.01*$AP*cp
                        if (caster->GetTypeId() != TYPEID_PLAYER)
                            return;

                        uint8 cp = ((Player*)caster)->GetComboPoints();

                        // Idol of Feral Shadows. Cant be handled as SpellMod in SpellAura:Dummy due its dependency from CPs
                        if (AuraEffect const * aurEff = caster->GetAuraEffect(34241,0))
                            m_amount += cp * aurEff->GetAmount();

                        m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * cp / 100);
                    }
                    // TODO: i do not know what is this for so i simply disable it
                    // Lifebloom
                    //else if (m_spellProto->SpellFamilyFlags[1] & 0x10 && GetAuraName() == SPELL_AURA_PERIODIC_HEAL)
                    //    m_amount = caster->SpellHealingBonus(m_target, GetSpellProto(), m_amount, SPELL_DIRECT_DAMAGE);
                    // Innervate
                    else if (m_spellProto->Id == 29166 && GetAuraName() == SPELL_AURA_PERIODIC_ENERGIZE)
                        m_amount = m_target->GetCreatePowers(POWER_MANA) * m_amount / (GetTotalTicks() * 100.0f);
                    // Owlkin Frenzy
                    else if (m_spellProto->Id == 48391 && GetAuraName() == SPELL_AURA_PERIODIC_ENERGIZE)
                        m_amount = m_target->GetCreatePowers(POWER_MANA) * m_amount / 100.0f;
                    // Thorns
                    else if (m_spellProto->SpellFamilyFlags[0] & 0x100 && GetAuraName() == SPELL_AURA_DAMAGE_SHIELD)
                        // 3.3% from sp bonus
                        DoneActualBenefit = caster->SpellBaseDamageBonus(GetSpellSchoolMask(m_spellProto)) * 0.033f;
                    break;
                }
                case SPELLFAMILY_ROGUE:
                {
                    // Rupture
                    if (m_spellProto->SpellFamilyFlags[0] & 0x100000 && GetAuraName() == SPELL_AURA_PERIODIC_DAMAGE)
                    {
                        if (caster->GetTypeId() != TYPEID_PLAYER)
                            return;
                        //1 point : ${($m1+$b1*1+0.015*$AP)*4} damage over 8 secs
                        //2 points: ${($m1+$b1*2+0.024*$AP)*5} damage over 10 secs
                        //3 points: ${($m1+$b1*3+0.03*$AP)*6} damage over 12 secs
                        //4 points: ${($m1+$b1*4+0.03428571*$AP)*7} damage over 14 secs
                        //5 points: ${($m1+$b1*5+0.0375*$AP)*8} damage over 16 secs
                        float AP_per_combo[6] = {0.0f, 0.015f, 0.024f, 0.03f, 0.03428571f, 0.0375f};
                        uint8 cp = ((Player*)caster)->GetComboPoints();
                        if (cp > 5) cp = 5;
                        m_amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * AP_per_combo[cp]);
                    }
                    break;
                }
                case SPELLFAMILY_PALADIN:
                    // Sacred Shield
                    if (m_spellProto->SpellFamilyFlags[1] & 0x80000 && GetAuraName() == SPELL_AURA_SCHOOL_ABSORB)
                    {
                        // 0.75 from sp bonus
                        float koef = 1.0f;
                        if (caster->HasAura(53527))
                            koef = 1.1f;
                        if (caster->HasAura(53530))
                            koef = 1.2f;

                        DoneActualBenefit = caster->SpellBaseHealingBonus(GetSpellSchoolMask(m_spellProto)) * 0.75f * koef;
                    }
                    break;
                case SPELLFAMILY_SHAMAN:
                    // Earth Shield
                    if (m_spellProto->SpellFamilyFlags[1] & 0x400 && GetAuraName() == SPELL_AURA_DUMMY)
                        m_amount = caster->SpellHealingBonus(m_target, GetSpellProto(), m_amount, SPELL_DIRECT_DAMAGE);
                    break;
                case SPELLFAMILY_DEATHKNIGHT:
                {
                    // Vampiric Blood
                    if(GetSpellProto()->Id == 55233 && GetAuraName() == SPELL_AURA_MOD_INCREASE_HEALTH)
                        m_amount = m_target->GetMaxHealth() * m_amount / 100;
                    // Icebound Fortitude
                    else if (m_spellProto->SpellFamilyFlags[0] & 0x00100000 && GetAuraName() == SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN)
                    {
                        if (caster->GetTypeId() == TYPEID_PLAYER)
                        {
                            int32 value = int32((m_amount*-1)-10);
                            uint32 defva = uint32(((Player*)caster)->GetSkillValue(SKILL_DEFENSE) + ((Player*)caster)->GetRatingBonusValue(CR_DEFENSE_SKILL));

                            if(defva > 400)
                                value += int32((defva-400)*0.15);

                            // Glyph of Icebound Fortitude
                            if (AuraEffect *auradummy = caster->GetAuraEffect(58625,0))
                            {
                                uint32 valmax = auradummy->GetAmount();
                                if(value < valmax)
                                    value = valmax;
                            }
                            m_amount = -value;
                        }
                    }
                    break;
                }
                default:
                    break;
            }

            if (DoneActualBenefit != 0.0f)
            {
                DoneActualBenefit *= caster->CalculateLevelPenalty(GetSpellProto());
                m_amount += (int32)DoneActualBenefit;
            }
        }
    }
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
    data << uint8(m_stackAmount > 1 ? m_stackAmount : (m_procCharges) ? m_procCharges : 1);

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

bool Aura::IsVisible() const
{
    // passive auras (except totem auras) do not get placed in the slots
    // area auras with SPELL_AURA_NONE are not shown on target
    //(m_spellProto->Attributes & 0x80 && GetTalentSpellPos(GetId()))

    if(!m_isPassive)
        return true;

    bool noneAreaAura = true;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if(m_partAuras[i])
        {
            if(m_partAuras[i]->IsAreaAura())
            {
                if(Unit *source = ((AreaAuraEffect*)m_partAuras[i])->GetSource())
                    if(source->isTotem())
                        return true;

                if(m_partAuras[i]->GetAuraName() != SPELL_AURA_NONE)
                    noneAreaAura = false;
            }
            else
                noneAreaAura = false;
        }
    }

    if(noneAreaAura)
        return false;

    return IsAuraType(SPELL_AURA_ABILITY_IGNORE_AURASTATE);
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

    if(IsVisible())
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
            for (uint32 freeSlot = 0; freeSlot < MAX_AURAS; ++itr , ++freeSlot)
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
            for (std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
            {
                if(*itr < 0)
                    m_target->ApplySpellImmune(id, IMMUNITY_ID, -(*itr), true);
                else if(Unit* caster = GetCaster())
                    caster->AddAura(*itr, m_target);
            }
    }

    HandleAuraSpecificMods(true);
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
    uint32 id = GetId();
    // Remove Linked Auras
    if(m_removeMode != AURA_REMOVE_BY_STACK && m_removeMode != AURA_REMOVE_BY_DEATH)
    {
        if(uint32 customAttr = spellmgr.GetSpellCustomAttr(id))
        {
            if(customAttr & SPELL_ATTR_CU_LINK_REMOVE)
            {
                if(const std::vector<int32> *spell_triggered = spellmgr.GetSpellLinked(-(int32)id))
                    for (std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
                    {
                        if(*itr < 0)
                            m_target->RemoveAurasDueToSpell(-(*itr));
                        else if (m_removeMode != AURA_REMOVE_BY_DEFAULT)
                            m_target->CastSpell(m_target, *itr, true, 0, 0, GetCasterGUID());
                    }
            }
            if(customAttr & SPELL_ATTR_CU_LINK_AURA)
            {
                if(const std::vector<int32> *spell_triggered = spellmgr.GetSpellLinked(id + SPELL_LINK_AURA))
                    for (std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
                    {
                        if(*itr < 0)
                            m_target->ApplySpellImmune(id, IMMUNITY_ID, -(*itr), false);
                        else
                            m_target->RemoveAurasDueToSpell(*itr);
                    }
            }
        }
    }

    // Proc on aura remove (only spell flags for now)
    if (caster)
    {
        uint32 procEx=0;
        if (m_removeMode == AURA_REMOVE_BY_ENEMY_SPELL)
            procEx = PROC_EX_AURA_REMOVE_DESTROY;
        else if (m_removeMode == AURA_REMOVE_BY_EXPIRE || m_removeMode == AURA_REMOVE_BY_CANCEL)
            procEx = PROC_EX_AURA_REMOVE_EXPIRE;
        if (procEx)
        {
            uint32 ProcCaster, ProcVictim;
            if (IsPositive())
            {
                ProcCaster = PROC_FLAG_SUCCESSFUL_POSITIVE_MAGIC_SPELL | PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL_HIT;
                ProcVictim = PROC_FLAG_TAKEN_POSITIVE_MAGIC_SPELL | PROC_FLAG_TAKEN_POSITIVE_SPELL;
            }
            else
            {
                ProcCaster = PROC_FLAG_SUCCESSFUL_NEGATIVE_MAGIC_SPELL | PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT;
                ProcVictim = PROC_FLAG_TAKEN_NEGATIVE_MAGIC_SPELL | PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT;
            }
            caster->ProcDamageAndSpell(m_target,ProcCaster, ProcVictim, procEx, m_procDamage, BASE_ATTACK, m_spellProto);
        }
    }
    HandleAuraSpecificMods(false);
}

void Aura::SetStackAmount(uint8 stackAmount, bool applied)
{
    bool refresh = stackAmount >= m_stackAmount;
    if (stackAmount != m_stackAmount)
    {
        m_stackAmount = stackAmount;
        for (uint8 i=0; i<MAX_SPELL_EFFECTS; ++i)
        {
            if (AuraEffect * part = GetPartAura(i))
            {
                part->RecalculateAmount(applied);
            }
        }
    }

    if (refresh)
        RefreshAura();
    else
        SendAuraUpdate();
}

// TODO: lifebloom should bloom when each stack is dispelled
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

void Aura::SetAuraDuration(int32 duration, bool withMods)
{
    if (withMods)
    {
        if (Player * modOwner = m_target->GetSpellModOwner())
            modOwner->ApplySpellMod(GetId(), SPELLMOD_DURATION, duration);
    }
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
    if(m_procCharges) //auras without charges always have charge = 0
    {
        if(--m_procCharges) // Send charge change
            SendAuraUpdate();
        else              // Last charge dropped
        {
            m_target->RemoveAura(this, AURA_REMOVE_BY_EXPIRE);
            return true;
        }
    }
    return false;
}

bool Aura::CanBeSaved() const
{
    if (IsPassive())
        return false;

    if(IsPersistent())
        return false;

    if (GetCasterGUID() != m_target->GetGUID())
        if (IsSingleTargetSpell(GetSpellProto()) || IsAreaAura())
            return false;

    // Can't be saved - aura handler relies on calculated amount and changes it
    if (IsAuraType(SPELL_AURA_CONVERT_RUNE))
        return false;

    return true;
}

bool Aura::IsPersistent() const
{
    return IS_DYNAMICOBJECT_GUID(m_sourceGuid);
    /*
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if(m_partAuras[i] && m_partAuras[i]->IsPersistent())
            return true;
    return false;
    */
}

bool Aura::IsAreaAura() const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if(m_partAuras[i] && m_partAuras[i]->IsAreaAura())
            return true;
    return false;
}

bool Aura::IsAuraType(AuraType type) const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if(m_partAuras[i] && m_partAuras[i]->GetAuraName() == type)
            return true;
    }
    return false;
}

void Aura::SetLoadedState(uint64 caster_guid,int32 maxduration,int32 duration,int32 charges, uint8 stackamount, int32 * amount)
{
    *const_cast<uint64*>(&m_casterGuid) = caster_guid;
    m_maxduration = maxduration;
    m_duration = duration;
    m_procCharges = charges;
    m_stackAmount = stackamount;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if(m_partAuras[i])
            m_partAuras[i]->SetAmount(amount[i]);
}

void AuraEffect::HandleShapeshiftBoosts(bool apply)
{
    uint32 spellId = 0;
    uint32 spellId2 = 0;
    uint32 spellId3 = 0;
    uint32 HotWSpellId = 0;

    switch(GetMiscValue())
    {
        case FORM_CAT:
            spellId = 3025;
            HotWSpellId = 24900;
            break;
        case FORM_TREE:
            spellId = 34123;
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
        case FORM_SHADOW:
            spellId = 49868;
            break;
        case FORM_GHOSTWOLF:
            spellId = 67116;
            break;
        case FORM_GHOUL:
        case FORM_AMBIENT:
        case FORM_STEALTH:
        case FORM_CREATURECAT:
        case FORM_CREATUREBEAR:
            break;
    }

    uint32 form = GetMiscValue()-1;

    if(apply)
    {
        // Remove cooldown of spells triggered on stance change - they may share cooldown with stance spell
        if (spellId)
        {
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                ((Player *)m_target)->RemoveSpellCooldown(spellId);
            m_target->CastSpell(m_target, spellId, true, NULL, this );
        }

        if (spellId2)
        {
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                ((Player *)m_target)->RemoveSpellCooldown(spellId2);
            m_target->CastSpell(m_target, spellId2, true, NULL, this);
        }

        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            const PlayerSpellMap& sp_list = ((Player *)m_target)->GetSpellMap();
            for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
            {
                if(itr->second->state == PLAYERSPELL_REMOVED) continue;
                if(itr->first==spellId || itr->first==spellId2) continue;
                SpellEntry const *spellInfo = sSpellStore.LookupEntry(itr->first);
                if (!spellInfo || !(spellInfo->Attributes & (SPELL_ATTR_PASSIVE | (1<<7)))) continue;
                if (spellInfo->Stances & (1<<(form)))
                    m_target->CastSpell(m_target, itr->first, true, NULL, this);
            }
            // Leader of the Pack
            if (((Player*)m_target)->HasSpell(17007))
            {
                SpellEntry const *spellInfo = sSpellStore.LookupEntry(24932);
                if (spellInfo && spellInfo->Stances & (1<<(form)))
                    m_target->CastSpell(m_target, 24932, true, NULL, this);
            }
            // Improved Barkskin - apply/remove armor bonus due to shapeshift
            if (((Player*)m_target)->HasSpell(63410) || ((Player*)m_target)->HasSpell(63411))
            {
                m_target->RemoveAurasDueToSpell(66530);
                if (form == FORM_TRAVEL || form == FORM_NONE) // "while in Travel Form or while not shapeshifted"
                    m_target->CastSpell(m_target, 66530, true);
            }
            // Heart of the Wild
            if (HotWSpellId)
            {
                Unit::AuraEffectList const& mModTotalStatPct = m_target->GetAurasByType(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE);
                for (Unit::AuraEffectList::const_iterator i = mModTotalStatPct.begin(); i != mModTotalStatPct.end(); ++i)
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
            switch(GetMiscValue())
            {
                case FORM_CAT:
                    // Nurturing Instinct
                    if (AuraEffect const * aurEff = m_target->GetAuraEffect(SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT, SPELLFAMILY_DRUID, 2254,0))
                    {
                        uint32 spellId = 0;
                        switch (aurEff->GetId())
                        {
                        case 33872:
                            spellId = 47179;
                            break;
                        case 33873:
                            spellId = 47180;
                            break;
                        }
                        m_target->CastSpell(m_target, spellId, true, NULL, this);
                    }
                    // Master Shapeshifter - Cat
                    if (AuraEffect const * aurEff = m_target->GetDummyAura(SPELLFAMILY_GENERIC, 2851, 0))
                    {
                        int32 bp = aurEff->GetAmount();
                        m_target->CastCustomSpell(m_target, 48420, &bp, NULL, NULL, true);
                    }
                break;
                case FORM_DIREBEAR:
                case FORM_BEAR:
                    // Master Shapeshifter - Bear
                    if (AuraEffect const * aurEff = m_target->GetDummyAura(SPELLFAMILY_GENERIC, 2851, 0))
                    {
                        int32 bp = aurEff->GetAmount();
                        m_target->CastCustomSpell(m_target, 48418, &bp, NULL, NULL, true);
                    }
                    // Survival of the Fittest
                    if (AuraEffect const * aurEff = m_target->GetAuraEffect(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE,SPELLFAMILY_DRUID, 961, 0))
                    {
                        int32 bp = m_target->CalculateSpellDamage(aurEff->GetSpellProto(),2,aurEff->GetSpellProto()->EffectBasePoints[2],m_target);
                        m_target->CastCustomSpell(m_target, 62069,&bp, NULL, NULL, true, 0, this);
                    }
                break;
                case FORM_MOONKIN:
                    // Master Shapeshifter - Moonkin
                    if (AuraEffect const * aurEff = m_target->GetDummyAura(SPELLFAMILY_GENERIC, 2851, 0))
                    {
                        int32 bp = aurEff->GetAmount();
                        m_target->CastCustomSpell(m_target, 48421, &bp, NULL, NULL, true);
                    }
                break;
                    // Master Shapeshifter - Tree of Life
                case FORM_TREE:
                    if (AuraEffect const * aurEff = m_target->GetDummyAura(SPELLFAMILY_GENERIC, 2851, 0))
                    {
                        int32 bp = aurEff->GetAmount();
                        m_target->CastCustomSpell(m_target, 48422, &bp, NULL, NULL, true);
                    }
                break;
            }
        }
    }
    else
    {
        if (spellId)
            m_target->RemoveAurasDueToSpell(spellId);
        if (spellId2)
            m_target->RemoveAurasDueToSpell(spellId2);

        // Improved Barkskin - apply/remove armor bonus due to shapeshift
        if (((Player*)m_target)->HasSpell(63410) || ((Player*)m_target)->HasSpell(63411))
        {
            m_target->RemoveAurasDueToSpell(66530);
            m_target->CastSpell(m_target,66530,true);
        }

        Unit::AuraMap& tAuras = m_target->GetAuras();
        for (Unit::AuraMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
        {
            if (itr->second->IsRemovedOnShapeLost())
                m_target->RemoveAura(itr);
            else
                ++itr;
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
            sLog.outCrash("Couldn't find the caster (guid: "UI64FMTD") of the single target aura %u which is on unit entry %u class %u, may crash later!", GetCasterGUID(), GetId(), m_target->GetEntry(), m_target->getClass());
            assert(false);
        }
        m_isSingleTargetAura = false;
    }
}

/*********************************************************/
/***               BASIC AURA FUNCTION                 ***/
/*********************************************************/
void AuraEffect::HandleAddModifier(bool apply, bool Real, bool changeAmount)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER || (!Real && !changeAmount))
        return;

    uint32 modOp = GetMiscValue();

    if(modOp >= MAX_SPELLMOD)
        return;

    if (apply)
    {
        SpellModifier *mod = new SpellModifier(GetParentAura());
        mod->op = SpellModOp(modOp);
        mod->value = m_amount;
        mod->type = SpellModType(m_auraName);    // SpellModType value == spell aura types
        mod->spellId = GetId();
        mod->mask = m_spellProto->EffectSpellClassMask[m_effIndex];
        mod->charges = GetParentAura()->GetAuraCharges();

        m_spellmod = mod;
    }

    ((Player*)m_target)->AddSpellMod(m_spellmod, apply);

    // Auras with charges do not mod amount of passive auras
    if (GetParentAura()->GetAuraCharges())
        return;
    // reapply some passive spells after add/remove related spellmods
    // Warning: it is a dead loop if 2 auras each other amount-shouldn't happen
    switch (modOp)
    {
        case SPELLMOD_ALL_EFFECTS:
        case SPELLMOD_EFFECT1:
        case SPELLMOD_EFFECT2:
        case SPELLMOD_EFFECT3:
        {
            uint64 guid = m_target->GetGUID();
            Unit::AuraMap & auras = m_target->GetAuras();
            for (Unit::AuraMap::iterator iter = auras.begin(); iter != auras.end(); ++iter)
            {
                Aura * aur = iter->second;
                // only passive auras-active auras should have amount set on spellcast and not be affected
                // if aura is casted by others, it will not be affected
                if (aur->IsPassive() && aur->GetCasterGUID() == guid && spellmgr.IsAffectedByMod(aur->GetSpellProto(), m_spellmod))
                {
                    if (modOp == SPELLMOD_ALL_EFFECTS)
                    {
                        for (uint8 i = 0; i<MAX_SPELL_EFFECTS; ++i)
                        {
                            if (AuraEffect * aurEff = aur->GetPartAura(i))
                                aurEff->RecalculateAmount();
                        }
                    }
                    else if (modOp ==SPELLMOD_EFFECT1)
                    {
                       if (AuraEffect * aurEff = aur->GetPartAura(0))
                            aurEff->RecalculateAmount();
                    }
                    else if (modOp ==SPELLMOD_EFFECT2)
                    {
                       if (AuraEffect * aurEff = aur->GetPartAura(1))
                            aurEff->RecalculateAmount();
                    }
                    else //if (modOp ==SPELLMOD_EFFECT3)
                    {
                       if (AuraEffect * aurEff = aur->GetPartAura(2))
                            aurEff->RecalculateAmount();
                    }
                }
            }
        }
        default:
            break;
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
                        if (caster->GetTypeId() != TYPEID_PLAYER)
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
                    // Thaumaturgy Channel
                    case 9712: trigger_spell_id = 21029; break;
                    case 23170:
                    {
                        m_target->CastSpell(m_target, 23171, true, 0, this);
                        return;
                    }
                    // Restoration
                    case 23493:
                    {
                        int32 heal = caster->GetMaxHealth() / 10;
                        caster->DealHeal(m_target, heal, auraSpellInfo);

                        int32 mana = caster->GetMaxPower(POWER_MANA);
                        if (mana)
                        {
                            mana /= 10;
                            caster->EnergizeBySpell(caster, 23493, mana, POWER_MANA);
                        }
                        return;
                    }
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
                    // Frost Blast
                    case 27808:
                        caster->CastCustomSpell(29879, SPELLVALUE_BASE_POINT0, (float)m_target->GetMaxHealth()*0.26f, m_target, true, NULL, this);
                        return;
                    // Detonate Mana
                    case 27819:
                        if(int32 mana = (int32)(m_target->GetMaxPower(POWER_MANA) / 4))
                        {
                            mana = m_target->ModifyPower(POWER_MANA, -mana);
                            m_target->CastCustomSpell(27820, SPELLVALUE_BASE_POINT0, -mana*4, NULL, true, NULL, this, caster->GetGUID());
                        }
                        return;
                    // Inoculate Nestlewood Owlkin
                    case 29528:
                        if(target->GetTypeId()!=TYPEID_UNIT)// prevent error reports in case ignored player target
                            return;
                        break;
                    // Feed Captured Animal
                    case 29917: trigger_spell_id = 29916; break;
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

                            creature->ForcedDespawn();
                        }
                        return;
                    }
                    // Quake
                    case 30576: trigger_spell_id = 30571; break;
                    // Doom
                    case 31347:
                    {
                        m_target->CastSpell(m_target,31350,true, NULL, this);
                        m_target->Kill(m_target);
                        return;
                    }
                    // Spellcloth
                    case 31373:
                    {
                        // Summon Elemental after create item
                        caster->SummonCreature(17870, 0, 0, 0, caster->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
                        return;
                    }
                    // Flame Quills
                    case 34229:
                    {
                        // cast 24 spells 34269-34289, 34314-34316
                        for (uint32 spell_id = 34269; spell_id != 34290; ++spell_id)
                            caster->CastSpell(m_target,spell_id,true, NULL, this);
                        for (uint32 spell_id = 34314; spell_id != 34317; ++spell_id)
                            caster->CastSpell(m_target,spell_id,true, NULL, this);
                        return;
                    }
                    // Remote Toy
                    case 37027: trigger_spell_id = 37029; break;
                    // Eye of Grillok
                    case 38495:
                    {
                        m_target->CastSpell(m_target, 38530, true, NULL, this);
                        return;
                    }
                    // Absorb Eye of Grillok (Zezzak's Shard)
                    case 38554:
                    {
                        if(m_target->GetTypeId() != TYPEID_UNIT)
                            return;

                        caster->CastSpell(caster, 38495, true, NULL, this);

                        Creature* creatureTarget = (Creature*)m_target;

                        creatureTarget->ForcedDespawn();
                        return;
                    }
                    // Tear of Azzinoth Summon Channel - it's not really supposed to do anything,and this only prevents the console spam
                    case 39857: trigger_spell_id = 39856; break;
                    // Aura of Desire
                    case 41350:
                    {
                        AuraEffect * aurEff = GetParentAura()->GetPartAura(1);
                        aurEff->ApplyModifier(false, false, true);
                        aurEff->SetAmount(aurEff->GetAmount()-5);
                        aurEff->ApplyModifier(true, false, true);
                        break;
                    }
                    case 46736: trigger_spell_id = 46737; break;
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
            case SPELLFAMILY_DRUID:
            {
                switch(auraId)
                {
                    // Cat Form
                    // trigger_spell_id not set and unknown effect triggered in this case, ignoring for while
                    case 768:
                        return;
                    default:
                        break;
                }
                break;
            }
            case SPELLFAMILY_HUNTER:
            {
                switch (auraId)
                {
                    // Sniper training
                    case 53302:
                    case 53303:
                    case 53304:
                        if (m_target->GetTypeId() != TYPEID_PLAYER)
                            return;

                        if (((Player*)m_target)->isMoving())
                        {
                            m_amount = m_target->CalculateSpellDamage(m_spellProto,m_effIndex,m_currentBasePoints,m_target);
                            return;
                        }

                        // We are standing at the moment
                        if (m_amount > 0)
                        {
                            --m_amount;
                            return;
                        }

                        trigger_spell_id = 64418 + auraId - 53302;

                        // If aura is active - no need to continue
                        if (target->HasAura(trigger_spell_id))
                            return;

                        break;
                    default:
                        break;
                }
                break;
            }
            case SPELLFAMILY_SHAMAN:
            {
                switch(auraId)
                {
                    // Lightning Shield (The Earthshatterer set trigger after cast Lighting Shield)
                    case 28820:
                    {
                        // Need remove self if Lightning Shield not active
                        if (!target->GetAura(SPELL_AURA_PROC_TRIGGER_SPELL, SPELLFAMILY_SHAMAN,0x400))
                            target->RemoveAurasDueToSpell(28820);
                        return;
                    }
                    // Totemic Mastery (Skyshatter Regalia (Shaman Tier 6) - bonus)
                    case 38443:
                    {
                        bool all = true;
                        for (int i = SUMMON_SLOT_TOTEM; i < MAX_TOTEM_SLOT; ++i)
                        {
                            if(!caster->m_SummonSlot[i])
                            {
                                all = false;
                                break;
                            }
                        }

                        if(all)
                            caster->CastSpell(caster,38437,true, NULL, this);
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
            // Mana Tide
            case 16191:
            {
                caster->CastCustomSpell(target, trigger_spell_id, &m_amount, NULL, NULL, true, NULL, this);
                return;
            }
            // Negative Energy Periodic
            case 46284:
                caster->CastCustomSpell(trigger_spell_id, SPELLVALUE_MAX_TARGETS, m_tickNumber / 10 + 1, NULL, true, NULL, this);
                return;
            // Poison (Grobbulus)
            case 28158:
            case 54362:
                m_target->CastCustomSpell(trigger_spell_id, SPELLVALUE_RADIUS_MOD, (int32)((((float)m_tickNumber / 60) * 0.9f + 0.1f) * 10000), NULL, true, NULL, this);
                return;
            // Mind Sear (target 76/16) if let m_target cast, will damage caster
            case 48045:
            case 53023:
            // Curse of the Plaguebringer (22/15)
            case 29213:
            case 54835:
                caster->CastSpell(m_target, trigger_spell_id, true, NULL, this);
                return;
            // Ground Slam
            case 33525:
                target->CastSpell(target, trigger_spell_id, true);
                return;
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

void AuraEffect::HandleAuraDummy(bool apply, bool Real, bool changeAmount)
{
    Unit* caster = GetCaster();

    // spells required only Real aura add/remove
    if (Real)
    {
    // AT APPLY
    if(apply)
    {
        // Overpower
        if (caster && m_spellProto->SpellFamilyName == SPELLFAMILY_WARRIOR &&
            m_spellProto->SpellFamilyFlags[0] & 0x4)
        {
            // Must be casting target
            if (!m_target->IsNonMeleeSpellCasted(false, false, true))
                return;
            if (AuraEffect * aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_WARRIOR, 2775, 0))
            {
                switch (aurEff->GetId())
                {
                    // Unrelenting Assault, rank 1
                    case 46859:
                        m_target->CastSpell(m_target,64849,true,NULL,aurEff);
                        break;
                    // Unrelenting Assault, rank 2
                    case 46860:
                        m_target->CastSpell(m_target,64850,true,NULL,aurEff);
                        break;
                }
            }
            return;
        }
        switch(GetId())
        {
            // Haunting Spirits - perdiodic trigger demon
            case 7057:
                m_isPeriodic = true;
                m_amplitude = irand (0, 60) + 30;
                m_amplitude *= IN_MILISECONDS;
                return;
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
            case 37096:                                     // Blood Elf Illusion
            {
                if(caster)
                {
                    switch(caster->getGender())
                    {
                        case GENDER_FEMALE:
                            caster->CastSpell(m_target,37095,true,NULL,this); // Blood Elf Disguise
                            break;
                        case GENDER_MALE:
                            caster->CastSpell(m_target,37093,true,NULL,this);
                            break;
                        default:
                            break;
                    }
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
            case 49028:
                GetParentAura()->SetAuraDuration(GetParentAura()->GetAuraDuration() + (caster->GetPower(POWER_RUNIC_POWER) * 10));
                caster->SetPower(POWER_RUNIC_POWER, 0);
                break;
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
                if(m_target->GetTypeId() == TYPEID_PLAYER)
                    ((Player*)m_target)->RemoveAmmo();      // not use ammo and not allow use
                return;
            case 52916:  // Honor Among Thieves
                if(m_target->GetTypeId() == TYPEID_PLAYER)
                    if (Unit * target = ObjectAccessor::GetUnit(*m_target,((Player*)m_target)->GetComboTarget()))
                        m_target->CastSpell(target, 51699, true);
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

        switch(m_spellProto->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
                switch(GetId())
                {
                    case 2584: // Waiting to Resurrect
                        // Waiting to resurrect spell cancel, we must remove player from resurrect queue
                        if(m_target->GetTypeId() == TYPEID_PLAYER)
                            if(BattleGround *bg = ((Player*)m_target)->GetBattleGround())
                                bg->RemovePlayerFromResurrectQueue(m_target->GetGUID());
                        return;
                    case 36730:                                     // Flame Strike
                    {
                        m_target->CastSpell(m_target, 36731, true, NULL, this);
                        return;
                    }
                    case 44191:                                     // Flame Strike
                    {
                        if (m_target->GetMap()->IsDungeon())
                        {
                            uint32 spellId = ((InstanceMap*)m_target->GetMap())->GetDifficulty() == REGULAR_DIFFICULTY ? 44190 : 46163;

                            m_target->CastSpell(m_target, spellId, true, NULL, this);
                        }
                        return;
                    }
                    case 42783: // Wrath of the Astromancer
                        m_target->CastSpell(m_target,m_amount,true,NULL,this);
                        return;
                    case 46308: // Burning Winds casted only at creatures at spawn
                        m_target->CastSpell(m_target,47287,true,NULL,this);
                        return;
                    case 52172:  // Coyote Spirit Despawn Aura
                    case 60244:  // Blood Parrot Despawn Aura
                        m_target->CastSpell((Unit*)NULL, GetAmount(), true, NULL, this);
                        return;
                }
                break;
            case SPELLFAMILY_MAGE:
                // Living Bomb
                if(m_spellProto->SpellFamilyFlags[1] & 0x20000)
                {
                    if(caster && (GetParentAura()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL || GetParentAura()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE))
                        caster->CastSpell(m_target, GetAmount(), true);
                    return;
                }
                break;
            case SPELLFAMILY_WARLOCK:
                // Haunt
                if(m_spellProto->SpellFamilyFlags[1] & 0x40000)
                {
                    int32 bp0 = GetParentAura()->GetProcDamage() * m_amount / 100;
                    if(caster)
                        caster->CastCustomSpell(caster, 48210, &bp0, 0, 0, true, NULL, this);
                    return;
                }
                break;
            case SPELLFAMILY_PALADIN:
            switch (GetSpellProto()->SpellIconID)
            {
                // Blessing of Sanctuary
                // Greater Blessing of Sanctuary
                case 19:
                case 1804:
                {
                    if (!caster || !m_target)
                        return;

                    if (apply)
                    {
                        // Greater Blessing of Sanctuary does not provide strength bonus
                        int32 bp1 = 0;
                        caster->CastCustomSpell(m_target, 67480, NULL, (GetSpellProto()->SpellIconID == 1804) ? &bp1 : NULL, NULL, true);
                    }
                    else
                        m_target->RemoveAurasDueToSpell(67480);

                    return;
                }
            }
            break;
            case SPELLFAMILY_PRIEST:
                // Vampiric Touch
                if (m_spellProto->SpellFamilyFlags[1] & 0x0400 && GetParentAura()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                {
                    if (AuraEffect const * aurEff = GetParentAura()->GetPartAura(1))
                    {
                        int32 damage = aurEff->GetAmount()*4;
                        // backfire damage
                        m_target->CastCustomSpell(m_target, 64085, &damage, NULL, NULL, true, NULL, NULL,GetCasterGUID());
                    }
                }
                break;
            case SPELLFAMILY_HUNTER:
                // Misdirection
                if(GetId()==34477)
                {
                    m_target->SetReducedThreatPercent(0, 0);
                    return;
                }
                break;
            case SPELLFAMILY_DEATHKNIGHT:
                // Summon Gargoyle ( will start feeding gargoyle )
                if(GetId()==61777)
                {
                    m_target->CastSpell(m_target,m_spellProto->EffectTriggerSpell[m_effIndex],true);
                    return;
                }
                break;
            default:
                break;
        }
    }
    }

    // AT APPLY & REMOVE

    switch(m_spellProto->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            if (!Real)
                break;
            switch(GetId())
            {
                // Recently Bandaged
                case 11196:
                    m_target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, GetMiscValue(), apply);
                    return;
                // Unstable Power
                case 24658:
                {
                    uint32 spellId = 24659;
                    if (apply && caster)
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
                case 24661:
                {
                    uint32 spellId = 24662;
                    if (apply && caster)
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
                case 40133:
                {
                    if (!caster)
                        return;

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
                case 40132 :
                {
                    if (!caster)
                        return;

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
                case 57723: // Exhaustion
                case 57724: // Sated
                {
                    switch(GetId())
                    {
                        case 57723: m_target->ApplySpellImmune(GetId(), IMMUNITY_ID, 32182, apply); break; // Heroism
                        case 57724: m_target->ApplySpellImmune(GetId(), IMMUNITY_ID, 2825, apply);  break; // Bloodlust
                    }
                    return;
                }
                case 57819: // Argent Champion
                case 57820: // Ebon Champion
                case 57821: // Champion of the Kirin Tor
                case 57822: // Wyrmrest Champion
                {
                    if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    uint32 FactionID = 0;

                    if(apply)
                    {
                        switch(m_spellProto->Id)
                        {
                            case 57819: FactionID = 1106; break; // Argent Crusade
                            case 57820: FactionID = 1098; break; // Knights of the Ebon Blade
                            case 57821: FactionID = 1090; break; // Kirin Tor
                            case 57822: FactionID = 1091; break; // The Wyrmrest Accord
                        }
                    }
                    ((Player*)caster)->SetChampioningFaction(FactionID);
                    return;
                }
                // LK Intro VO (1)
                case 58204:
                    if(m_target->GetTypeId() == TYPEID_PLAYER)
                    {
                        // Play part 1
                        if(apply)
                            m_target->PlayDirectSound(14970, (Player *)m_target);
                        // continue in 58205
                        else
                            m_target->CastSpell(m_target, 58205, true);
                    }
                    return;
                // LK Intro VO (2)
                case 58205:
                    if(m_target->GetTypeId() == TYPEID_PLAYER)
                    {
                        // Play part 2
                        if(apply)
                            m_target->PlayDirectSound(14971, (Player *)m_target);
                        // Play part 3
                        else
                            m_target->PlayDirectSound(14972, (Player *)m_target);
                    }
                    return;
            }

            break;
        }
        case SPELLFAMILY_MAGE:
        {
            //if (!Real)
                //break;
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            if (!Real && !changeAmount)
                break;
            // Pain and Suffering
            if( m_spellProto->SpellIconID == 2874 && m_target->GetTypeId() == TYPEID_PLAYER )
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
            if (!Real && !changeAmount)
                break;
            switch(GetId())
            {
                case 34246:                                 // Idol of the Emerald Queen
                case 60779:                                 // Idol of Lush Moss
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
                    if (!Real)
                        break;
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
            // Predatory Strikes
            if(m_target->GetTypeId() == TYPEID_PLAYER && GetSpellProto()->SpellIconID == 1563)
            {
                ((Player*)m_target)->UpdateAttackPowerAndDamage();
                return;
            }
            if (!Real)
                break;
            // Lifebloom
            if ( GetSpellProto()->SpellFamilyFlags[1] & 0x10 )
            {
                if (!apply)
                {
                    // Final heal only on dispelled or duration end
                    if (GetParentAura()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE && GetParentAura()->GetRemoveMode() != AURA_REMOVE_BY_ENEMY_SPELL)
                        return;

                    // final heal
                    //if(m_target->IsInWorld())
                    // This may be a hack, but we need a way to count healing bonus three times
                    //for (uint8 i = 0; i < GetParentAura()->GetStackAmount(); ++i)
                    // Update: apparently not anymore, it should only count once, so.. commented out.
                    m_target->CastCustomSpell(m_target,33778,&m_amount,NULL,NULL,true,NULL,this,GetCasterGUID());

                    // restore mana
                    if (caster)
                    {
                        int32 returnmana = (GetSpellProto()->ManaCostPercentage * caster->GetCreateMana() / 100) * GetParentAura()->GetStackAmount() / 2;
                        caster->CastCustomSpell(caster, 64372, &returnmana, NULL, NULL, true, NULL, this, GetCasterGUID());
                    }
                }
                return;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            if (!Real)
                break;
            // Sentry Totem
            if (GetId() == 6495 && caster->GetTypeId() == TYPEID_PLAYER)
            {
                if (apply)
                {
                    uint64 guid = caster->m_SummonSlot[3];
                    if (guid)
                    {
                        Creature *totem = caster->GetMap()->GetCreature(guid);
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

    if (Real)
    {
        // pet auras
        if(PetAura const* petSpell = spellmgr.GetPetAura(GetId(), m_effIndex))
        {
            if(apply)
                m_target->AddPetAura(petSpell);
            else
                m_target->RemovePetAura(petSpell);
            return;
        }

        if(GetEffIndex()==0 && m_target->GetTypeId() == TYPEID_PLAYER)
        {
            SpellAreaForAreaMapBounds saBounds = spellmgr.GetSpellAreaForAuraMapBounds(GetId());
            if(saBounds.first != saBounds.second)
            {
                uint32 zone, area;
                m_target->GetZoneAndAreaId(zone,area);

                for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
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
}

void AuraEffect::HandleAuraMounted(bool apply, bool Real, bool /*changeAmount*/)
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
        if (m_target->GetTypeId() == TYPEID_PLAYER)
            team = ((Player*)m_target)->GetTeam();

        uint32 display_id = objmgr.ChooseDisplayId(team,ci);
        CreatureModelInfo const *minfo = objmgr.GetCreatureModelRandomGender(display_id);
        if (minfo)
            display_id = minfo->modelid;

        //some spell has one aura of mount and one of vehicle
        for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if(GetSpellProto()->Effect[i] == SPELL_EFFECT_SUMMON
                && GetSpellProto()->EffectMiscValue[i] == GetMiscValue())
                display_id = 0;
        m_target->Mount(display_id);
    }
    else
    {
        m_target->Unmount();
        //some mounts like Headless Horseman's Mount or broom stick are skill based spell
        // need to remove ALL arura related to mounts, this will stop client crash with broom stick
        // and never endless flying after using Headless Horseman's Mount
        m_target->RemoveAurasByType(SPELL_AURA_MOUNTED);
    }
}

void AuraEffect::HandleAuraWaterWalk(bool apply, bool Real, bool /*changeAmount*/)
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

void AuraEffect::HandleAuraFeatherFall(bool apply, bool Real, bool /*changeAmount*/)
{
    // only at real add/remove aura
    if (!Real)
        return;

    if (!m_target)
        return;

    WorldPacket data;
    if (apply)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->GetGUID() == m_target->GetGUID())
        {
            m_target->RemoveAurasByType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED);
            m_target->RemoveAurasByType(SPELL_AURA_FLY);
        }

        if (GetId() == 61243) // No fly zone - Parachute
        {
            float x, y, z;
            caster->GetPosition(x, y, z);
            float ground_Z = caster->GetMap()->GetVmapHeight(x, y, z, true);
            if (fabs(ground_Z - z) < 0.1f)
            {
                m_target->RemoveAura(GetId());
                return;
            }
        }
        data.Initialize(SMSG_MOVE_FEATHER_FALL, 8+4);
    }
    else
        data.Initialize(SMSG_MOVE_NORMAL_FALL, 8+4);
    data.append(m_target->GetPackGUID());
    data << uint32(0);
    m_target->SendMessageToSet(&data, true);

    // start fall from current height
    if(!apply && m_target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_target)->SetFallInformation(0, m_target->GetPositionZ());
}

void AuraEffect::HandleAuraHover(bool apply, bool Real, bool /*changeAmount*/)
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

void AuraEffect::HandleWaterBreathing(bool apply, bool Real, bool /*changeAmount*/)
{
    // update timers in client
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_target)->UpdateMirrorTimers();
}

void AuraEffect::HandleAuraModShapeshift(bool apply, bool Real, bool changeAmount)
{
    if(!Real && !changeAmount)
        return;

    uint32 modelid = 0;
    Powers PowerType = POWER_MANA;
    ShapeshiftForm form = ShapeshiftForm(GetMiscValue());
    switch(form)
    {
        case FORM_CAT:
        case FORM_GHOUL:
            PowerType = POWER_ENERGY;
            break;
        case FORM_BEAR:
        case FORM_DIREBEAR:
        case FORM_BATTLESTANCE:
        case FORM_BERSERKERSTANCE:
        case FORM_DEFENSIVESTANCE:
            PowerType = POWER_RAGE;
            break;
        case FORM_TRAVEL:
        case FORM_AQUA:
        case FORM_CREATUREBEAR:
        case FORM_GHOSTWOLF:
        case FORM_FLIGHT:
        case FORM_MOONKIN:
        case FORM_FLIGHT_EPIC:
        case FORM_METAMORPHOSIS:
        case FORM_MASTER_ANGLER:
        case FORM_AMBIENT:
        case FORM_SHADOW:
        case FORM_STEALTH:
        case FORM_UNDEAD:
        case FORM_SHADOW_DANCE:
        case FORM_TREE:
        case FORM_SPIRITOFREDEMPTION:
            break;
        default:
            sLog.outError("Auras: Unknown Shapeshift Type: %u", GetMiscValue());
    }
    modelid = m_target->GetModelForForm(form);

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
        {
            // remove movement affects
            m_target->RemoveMovementImpairingAuras();

            // and polymorphic affects
            if(m_target->IsPolymorphed())
                m_target->RemoveAurasDueToSpell(m_target->getTransForm());
            break;
        }
        default:
           break;
    }

    if (apply)
    {
        // remove other shapeshift before applying a new one
        if(m_target->m_ShapeShiftFormSpellId)
            m_target->RemoveAurasDueToSpell(m_target->m_ShapeShiftFormSpellId);

        m_target->SetByteValue(UNIT_FIELD_BYTES_2, 3, form);

        if (modelid > 0)
            m_target->SetDisplayId(modelid);

        if (PowerType != POWER_MANA)
        {
            uint32 oldPower = m_target->GetPower(PowerType);
            // reset power to default values only at power change
            if(m_target->getPowerType() != PowerType)
                m_target->setPowerType(PowerType);

            switch (form)
            {
                case FORM_CAT:
                case FORM_BEAR:
                case FORM_DIREBEAR:
                {
                    // get furor proc chance
                    uint32 FurorChance = 0;
                    if (AuraEffect const *dummy = m_target->GetDummyAura(SPELLFAMILY_DRUID, 238, 0))
                        FurorChance = std::max(dummy->GetAmount(), 0);

                    switch (GetMiscValue())
                    {
                        case FORM_CAT:
                        {
                            int32 basePoints = int32(std::min(oldPower, FurorChance));
                            m_target->CastCustomSpell(m_target, 17099, &basePoints, NULL, NULL, true, NULL, this);
                        }
                        break;
                        case FORM_BEAR:
                        case FORM_DIREBEAR:
                        if (urand(0,99) < FurorChance)
                        {
                            int32 basePoints = 100; // Not sure if 100 is correct basePoints, maybe it should be 10?
                            m_target->CastCustomSpell(m_target, 17057, &basePoints, NULL, NULL, true, NULL, this);
                        }
                        default:
                        {
                            uint32 newEnergy = std::min(m_target->GetPower(POWER_ENERGY), FurorChance);
                            m_target->SetPower(POWER_ENERGY, newEnergy);
                        }
                        break;
                    }
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
                if(AuraEffect* dummy = m_target->GetAuraEffect(37315, 0) )
                    m_target->CastSpell(m_target,37316,true,NULL,dummy);
                break;
            // Nordrassil Regalia - bonus
            case FORM_MOONKIN:
                if(AuraEffect* dummy = m_target->GetAuraEffect(37324, 0) )
                    m_target->CastSpell(m_target,37325,true,NULL,dummy);
                break;
            case FORM_BATTLESTANCE:
            case FORM_DEFENSIVESTANCE:
            case FORM_BERSERKERSTANCE:
            {
                uint32 Rage_val = 0;
                // Defensive Tactics
                if (form == FORM_DEFENSIVESTANCE)
                {
                    if (AuraEffect const * aurEff = m_target->IsScriptOverriden(m_spellProto,831))
                        Rage_val += aurEff->GetAmount() * 10;
                }
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

    // adding/removing linked auras
    // add/remove the shapeshift aura's boosts
    HandleShapeshiftBoosts(apply);

    if(m_target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_target)->InitDataForForm();

    if(m_target->getClass() == CLASS_DRUID)
    {
        if(form == FORM_CAT && apply)
        {
            // add dash if in cat-from
            Unit::AuraMap & auras = m_target->GetAuras();
            for (Unit::AuraMap::iterator iter = auras.begin(); iter != auras.end(); ++iter)
            {
                Aura * aur = iter->second;
                if (aur->GetSpellProto()->SpellFamilyName==SPELLFAMILY_DRUID && aur->GetSpellProto()->SpellFamilyFlags[2] & 0x8)
                {
                    m_target->HandleAuraEffect(aur->GetPartAura(0), true);
                }
            }
        }
        else // remove dash effect(not buff) if out of cat-from
        {
            if(AuraEffect * aurEff =m_target->GetAura(SPELL_AURA_MOD_INCREASE_SPEED, SPELLFAMILY_DRUID, 0, 0, 0x8))
                m_target->HandleAuraEffect(aurEff, false);
        }
    }
    if (m_target->GetTypeId() == TYPEID_PLAYER)
    {
        SpellShapeshiftEntry const *shapeInfo = sSpellShapeshiftStore.LookupEntry(form);
        // Learn spells for shapeshift form - no need to send action bars or add spells to spellbook
        for (uint8 i = 0; i<MAX_SHAPESHIFT_SPELLS; ++i)
        {
            if (!shapeInfo->stanceSpell[i])
                continue;
            if (apply)
                ((Player*)m_target)->AddTemporarySpell(shapeInfo->stanceSpell[i]);
            else
                ((Player*)m_target)->RemoveTemporarySpell(shapeInfo->stanceSpell[i]);
        }
    }
}

void AuraEffect::HandleAuraTransform(bool apply, bool Real, bool /*changeAmount*/)
{
    if (apply)
    {
        // special case (spell specific functionality)
        if(GetMiscValue()==0)
        {
            // player applied only
            if (m_target->GetTypeId() != TYPEID_PLAYER)
                return;

            switch (GetId())
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
                m_target->SetDisplayId(16358);              // pig pink ^_^
                sLog.outError("Auras: unknown creature id = %d (only need its modelid) Form Spell Aura Transform in Spell ID = %d", GetMiscValue(), GetId());
            }
            else
            {
                uint32 model_id;

                if (uint32 modelid = ci->GetRandomValidModelId())
                    model_id = modelid;                     // Will use the default model here

                // Polymorph (sheep)
                if (GetSpellProto()->SpellFamilyName == SPELLFAMILY_MAGE && GetSpellProto()->SpellIconID == 82 && GetSpellProto()->SpellVisual[0] == 12978)
                    if (Unit * caster = GetCaster())
                        if (caster->HasAura(52648))         // Glyph of the Penguin
                            model_id = 26452;

                m_target->SetDisplayId(model_id);

                // Dragonmaw Illusion (set mount model also)
                if(GetId()==42016 && m_target->GetMountID() && !m_target->GetAurasByType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED).empty())
                    m_target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID,16314);
            }
        }

        // update active transform spell only not set or not overwriting negative by positive case
        if (!m_target->getTransForm() || !IsPositiveSpell(GetId()) || IsPositiveSpell(m_target->getTransForm()))
            m_target->setTransForm(GetId());

        // polymorph case
        if (Real && m_target->GetTypeId() == TYPEID_PLAYER && m_target->IsPolymorphed())
        {
            // for players, start regeneration after 1s (in polymorph fast regeneration case)
            // only if caster is Player (after patch 2.4.2)
            if (IS_PLAYER_GUID(GetCasterGUID()) )
                ((Player*)m_target)->setRegenTimerCount(1*IN_MILISECONDS);

            //dismount polymorphed target (after patch 2.4.2)
            if (m_target->IsMounted())
                m_target->RemoveAurasByType(SPELL_AURA_MOUNTED);
        }
    }
    else
    {
        // ApplyModifier(true) will reapply it if need
        m_target->setTransForm(0);
        m_target->SetDisplayId(m_target->GetNativeDisplayId());

        // re-aplly some from still active with preference negative cases
        Unit::AuraEffectList const& otherTransforms = m_target->GetAurasByType(SPELL_AURA_TRANSFORM);
        if (!otherTransforms.empty())
        {
            // look for other transform auras
            AuraEffect* handledAura = *otherTransforms.begin();
            for (Unit::AuraEffectList::const_iterator i = otherTransforms.begin(); i != otherTransforms.end(); ++i)
            {
                // negative auras are preferred
                if (!IsPositiveSpell((*i)->GetSpellProto()->Id))
                {
                    handledAura = *i;
                    break;
                }
            }
            handledAura->ApplyModifier(true);
        }

        // Dragonmaw Illusion (restore mount model)
        if (GetId()==42016 && m_target->GetMountID()==16314)
        {
            if (!m_target->GetAurasByType(SPELL_AURA_MOUNTED).empty())
            {
                uint32 cr_id = m_target->GetAurasByType(SPELL_AURA_MOUNTED).front()->GetMiscValue();
                if(CreatureInfo const* ci = objmgr.GetCreatureTemplate(cr_id))
                {
                    uint32 team = 0;
                    if (m_target->GetTypeId() == TYPEID_PLAYER)
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

void AuraEffect::HandleForceReaction(bool apply, bool Real, bool changeAmount)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if(!Real && !changeAmount)
        return;

    Player* player = (Player*)m_target;

    uint32 faction_id = GetMiscValue();
    uint32 faction_rank = m_amount;

    player->GetReputationMgr().ApplyForceReaction(faction_id,ReputationRank(faction_rank),apply);
    player->GetReputationMgr().SendForceReactions();
}

void AuraEffect::HandleAuraModSkill(bool apply, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 prot=GetSpellProto()->EffectMiscValue[m_effIndex];
    int32 points = m_amount;

    ((Player*)m_target)->ModifySkillBonus(prot,(apply ? points: -points),m_auraName==SPELL_AURA_MOD_SKILL_TALENT);
    if(prot == SKILL_DEFENSE)
        ((Player*)m_target)->UpdateDefenseBonusesMod();
}

void AuraEffect::HandleChannelDeathItem(bool apply, bool Real, bool /*changeAmount*/)
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
             victim->GetTypeId() == TYPEID_UNIT && !((Player*)caster)->isAllowedToLoot((Creature*)victim)) )
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

void AuraEffect::HandleBindSight(bool apply, bool Real, bool /*changeAmount*/)
{
    if (!Real)
        return;
    Unit* caster = GetCaster();
    if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)caster)->SetViewpoint(m_target, apply);
}

void AuraEffect::HandleFarSight(bool apply, bool Real, bool /*changeAmount*/)
{
    //Handled by client
}

void AuraEffect::HandleAuraTrackCreatures(bool apply, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    m_target->SetUInt32Value(PLAYER_TRACK_CREATURES, apply ? ((uint32)1)<<(GetMiscValue()-1) : 0 );
}

void AuraEffect::HandleAuraTrackResources(bool apply, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    m_target->SetUInt32Value(PLAYER_TRACK_RESOURCES, apply ? ((uint32)1)<<(GetMiscValue()-1): 0 );
}

void AuraEffect::HandleAuraTrackStealthed(bool apply, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    m_target->ApplyModFlag(PLAYER_FIELD_BYTES,PLAYER_FIELD_BYTE_TRACK_STEALTHED,apply);
}

void AuraEffect::HandleAuraModScale(bool apply, bool Real, bool /*changeAmount*/)
{
    m_target->ApplyPercentModFloatValue(OBJECT_FIELD_SCALE_X,m_amount,apply);
}

void AuraEffect::HandleAuraModPetTalentsPoints(bool Apply, bool Real, bool changeAmount)
{
    if(!Real && !changeAmount)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate pet tlaent points
    if (Pet *pet = ((Player*)m_target)->GetPet())
        pet->InitTalentForLevel();
}

void AuraEffect::HandleModConfuse(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;

    //m_target->SetConfused(apply, GetCasterGUID(), GetId());
    m_target->SetControlled(apply, UNIT_STAT_CONFUSED);
}

void AuraEffect::HandleModFear(bool apply, bool Real, bool /*changeAmount*/)
{
    if (!Real)
        return;

    //m_target->SetFeared(apply, GetCasterGUID(), GetId());
    m_target->SetControlled(apply, UNIT_STAT_FLEEING);
}

void AuraEffect::HandleFeignDeath(bool apply, bool Real, bool /*changeAmount*/)
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
        Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(m_target, m_target, m_target->GetMap()->GetVisibilityDistance());
        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(m_target, targets, u_check);
        m_target->VisitNearbyObject(m_target->GetMap()->GetVisibilityDistance(), searcher);
        for (std::list<Unit*>::iterator iter = targets.begin(); iter != targets.end(); ++iter)
        {
            if(!(*iter)->hasUnitState(UNIT_STAT_CASTING))
                continue;

            for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
            {
                if((*iter)->GetCurrentSpell(i)
                && (*iter)->GetCurrentSpell(i)->m_targets.getUnitTargetGUID() == m_target->GetGUID())
                {
                    (*iter)->InterruptSpell(CurrentSpellTypes(i), false);
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
        if(GetCasterGUID()==m_target->GetGUID() && m_target->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            m_target->FinishSpell(CURRENT_GENERIC_SPELL, false);
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

void AuraEffect::HandleAuraModDisarm(bool apply, bool Real, bool /*changeAmount*/)
{
    if (!Real)
        return;
    AuraType type = AuraType(GetAuraName());

    //Prevent handling aura twice
    if(apply ? m_target->GetAurasByType(type).size() > 1 : m_target->HasAuraType(type))
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
    default:
        return;
    }

    if(!apply)
        m_target->RemoveFlag(field, flag);

    if (m_target->GetTypeId() == TYPEID_PLAYER)
    {
        // This is between the two because there is a check in _ApplyItemMods
        // we must make sure that flag is always removed when call that function
        // refer to DurabilityPointsLoss
        if(Item *pItem = ((Player*)m_target)->GetItemByPos( INVENTORY_SLOT_BAG_0, slot ))
            ((Player*)m_target)->_ApplyItemMods(pItem, slot, !apply);
    }

    if(apply)
        m_target->SetFlag(field, flag);

    if (m_target->GetTypeId() == TYPEID_UNIT && ((Creature*)m_target)->GetCurrentEquipmentId())
        m_target->UpdateDamagePhysical(attType);
}

void AuraEffect::HandleModStealth(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;

    if(apply)
    {
        // drop flag at stealth in bg
        m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

        m_target->SetStandFlags(UNIT_STAND_FLAGS_CREEP);
        if(m_target->GetTypeId() == TYPEID_PLAYER)
            m_target->SetFlag(PLAYER_FIELD_BYTES2, 0x2000);

        // apply only if not in GM invisibility (and overwrite invisibility state)
        if(m_target->GetVisibility() != VISIBILITY_OFF)
            m_target->SetVisibility(VISIBILITY_GROUP_STEALTH);
    }
    else if(!m_target->HasAuraType(SPELL_AURA_MOD_STEALTH)) // if last SPELL_AURA_MOD_STEALTH
    {
        m_target->RemoveStandFlags(UNIT_STAND_FLAGS_CREEP);
        if(m_target->GetTypeId() == TYPEID_PLAYER)
            m_target->RemoveFlag(PLAYER_FIELD_BYTES2, 0x2000);

        if(m_target->GetVisibility() != VISIBILITY_OFF)
            m_target->SetVisibility(VISIBILITY_ON);
    }
}

void AuraEffect::HandleInvisibility(bool apply, bool Real, bool /*changeAmount*/)
{
    if(apply)
    {
        m_target->m_invisibilityMask |= (1 << GetMiscValue());

        if(Real)
        {
            m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

            // apply glow vision
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                m_target->SetFlag(PLAYER_FIELD_BYTES2,PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW);

            m_target->SetToNotify();
        }
    }
    else
    {
        // recalculate value at modifier remove (current aura already removed)
        m_target->m_invisibilityMask = 0;
        Unit::AuraEffectList const& auras = m_target->GetAurasByType(SPELL_AURA_MOD_INVISIBILITY);
        for (Unit::AuraEffectList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
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

void AuraEffect::HandleInvisibilityDetect(bool apply, bool Real, bool /*changeAmount*/)
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
        for (Unit::AuraEffectList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
            m_target->m_detectInvisibilityMask |= (1 << GetMiscValue());
    }
    if(Real && m_target->GetTypeId() == TYPEID_PLAYER)
        m_target->SetToNotify();
}

void AuraEffect::HandleAuraModSilence(bool apply, bool Real, bool /*changeAmount*/)
{
    // only at real add/remove aura
    if(!Real)
        return;

    if(apply)
    {
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
        // Stop cast only spells vs PreventionType == SPELL_PREVENTION_TYPE_SILENCE
        for (uint32 i = CURRENT_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
            if (Spell* spell = m_target->GetCurrentSpell(CurrentSpellTypes(i)))
                if(spell->m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
                    // Stop spells on prepare or casting state
                    m_target->InterruptSpell(CurrentSpellTypes(i), false);
    }
    else
    {
        // Real remove called after current aura remove from lists, check if other similar auras active
        if(m_target->HasAuraType(SPELL_AURA_MOD_SILENCE))
            return;

        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
    }
}

void AuraEffect::HandleModThreat(bool apply, bool Real, bool changeAmount)
{
    // only at real add/remove aura
    if(!Real && !changeAmount)
        return;

    if (!m_target->isAlive())
        return;

    Unit* caster = GetCaster();

    if (!caster || !caster->isAlive())
        return;

    int level_diff = 0;
    int multiplier = 0;
    if (apply && !m_target->isBeingLoaded())
    {
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
    }

    if (m_target->GetTypeId() == TYPEID_PLAYER)
        for (int8 x=0; x < MAX_SPELL_SCHOOL; x++)
            if (GetMiscValue() & int32(1<<x))
                ApplyPercentModFloatVar(m_target->m_threatModifier[x], m_amount, apply);
}

void AuraEffect::HandleAuraModTotalThreat(bool apply, bool Real, bool changeAmount)
{
    // only at real add/remove aura
    if(!Real && !changeAmount)
        return;

    if (!m_target->isAlive() || m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    Unit* caster = GetCaster();

    if (!caster || !caster->isAlive())
        return;

    float threatMod = apply ? float(m_amount) : float(-m_amount);

    m_target->getHostilRefManager().threatAssist(caster, threatMod);
}

void AuraEffect::HandleModTaunt(bool apply, bool Real, bool /*changeAmount*/)
{
    // only at real add/remove aura
    if (!Real)
        return;

    if (!m_target->isAlive() || !m_target->CanHaveThreatList())
        return;

    Unit* caster = GetCaster();

    if (!caster || !caster->isAlive())
        return;

    if (apply)
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
void AuraEffect::HandleAuraModIncreaseSpeed(bool apply, bool Real, bool changeAmount)
{
    // all applied/removed only at real aura add/remove
    if(!Real && !changeAmount)
        return;

    if(apply) // Dash wont work if you are not in cat form
        if(m_spellProto->SpellFamilyName==SPELLFAMILY_DRUID && m_spellProto->SpellFamilyFlags[2] & 0x8 && m_target->m_form != FORM_CAT )
        {
            m_target->HandleAuraEffect(this, false);
            return;
        }

    m_target->UpdateSpeed(MOVE_RUN, true);
}

void AuraEffect::HandleAuraModIncreaseMountedSpeed(bool /*apply*/, bool Real, bool changeAmount)
{
    // all applied/removed only at real aura add/remove
    if(!Real && !changeAmount)
        return;

    m_target->UpdateSpeed(MOVE_RUN, true);
}

void AuraEffect::HandleAuraModIncreaseFlightSpeed(bool apply, bool Real, bool changeAmount)
{
    // all applied/removed only at real aura add/remove
    if(!Real && !changeAmount)
        return;

    // Enable Fly mode for flying mounts
    if (m_auraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED)
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
        if (m_target->GetTypeId() == TYPEID_PLAYER)
            m_target->ApplySpellImmune(GetId(),IMMUNITY_MECHANIC,MECHANIC_POLYMORPH,apply);

        // Dragonmaw Illusion (overwrite mount model, mounted aura already applied)
        if( apply && m_target->HasAuraEffect(42016,0) && m_target->GetMountID())
            m_target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID,16314);
    }

    m_target->UpdateSpeed(MOVE_FLIGHT, true);
}

void AuraEffect::HandleAuraModIncreaseSwimSpeed(bool /*apply*/, bool Real, bool changeAmount)
{
    // all applied/removed only at real aura add/remove
    if(!Real && !changeAmount)
        return;

    m_target->UpdateSpeed(MOVE_SWIM, true);
}

void AuraEffect::HandleAuraModDecreaseSpeed(bool apply, bool Real, bool changeAmount)
{
    // all applied/removed only at real aura add/remove
    if(!Real && !changeAmount)
        return;

    m_target->UpdateSpeed(MOVE_RUN, true);
    m_target->UpdateSpeed(MOVE_SWIM, true);
    m_target->UpdateSpeed(MOVE_FLIGHT, true);
    m_target->UpdateSpeed(MOVE_RUN_BACK, true);
    m_target->UpdateSpeed(MOVE_SWIM_BACK, true);
    m_target->UpdateSpeed(MOVE_FLIGHT_BACK, true);
}

void AuraEffect::HandleAuraModUseNormalSpeed(bool /*apply*/, bool Real, bool changeAmount)
{
    // all applied/removed only at real aura add/remove
    if(!Real && !changeAmount)
        return;

    m_target->UpdateSpeed(MOVE_RUN,  true);
    m_target->UpdateSpeed(MOVE_SWIM, true);
    m_target->UpdateSpeed(MOVE_FLIGHT,  true);
}

/*********************************************************/
/***                     IMMUNITY                      ***/
/*********************************************************/

void AuraEffect::HandleModStateImmunityMask(bool apply, bool Real, bool /*changeAmount*/)
{
    if (!Real)
        return;
    std::list <AuraType> immunity_list;
    if (GetMiscValue() & (1<<10))
        immunity_list.push_back(SPELL_AURA_MOD_STUN);
    if (GetMiscValue() & (1<<7))
        immunity_list.push_back(SPELL_AURA_MOD_DISARM);
    if (GetMiscValue() & (1<<1))
        immunity_list.push_back(SPELL_AURA_TRANSFORM);

    // These flag can be recognized wrong:
    if (GetMiscValue() & (1<<6))
        immunity_list.push_back(SPELL_AURA_MOD_DECREASE_SPEED);
    if (GetMiscValue() & (1<<0))
        immunity_list.push_back(SPELL_AURA_MOD_ROOT);
    if (GetMiscValue() & (1<<2))
        immunity_list.push_back(SPELL_AURA_MOD_CONFUSE);
    if (GetMiscValue() & (1<<9))
        immunity_list.push_back(SPELL_AURA_MOD_FEAR);

    // Patch 3.0.3 Bladestorm now breaks all snares and roots on the warrior when activated.
    // however not all mechanic specified in immunity
    if (apply && GetId()==46924)
    {
        m_target->RemoveAurasByType(SPELL_AURA_MOD_ROOT);
        m_target->RemoveAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
    }

    if(apply && GetSpellProto()->AttributesEx & SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY)
    {
        for (std::list <AuraType>::iterator iter = immunity_list.begin(); iter != immunity_list.end(); ++iter)
        {
            m_target->RemoveAurasByType(*iter);
        }
    }
    for (std::list <AuraType>::iterator iter = immunity_list.begin(); iter != immunity_list.end(); ++iter)
    {
        m_target->ApplySpellImmune(GetId(),IMMUNITY_STATE,*iter,apply);
    }
}

void AuraEffect::HandleModMechanicImmunity(bool apply, bool Real, bool /*changeAmount*/)
{
    if (!Real)
        return;
    uint32 mechanic;
    mechanic = 1 << GetMiscValue();

    //immune movement impairment and loss of control
    if(GetId()==42292 || GetId()==59752)
        mechanic=IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK;
    // Forbearance
    // in DBC wrong mechanic immune since 3.0.x
    else if (GetId() == 25771)
        mechanic = 1 << MECHANIC_IMMUNE_SHIELD;

    if (!mechanic)
        return;

    if(apply && GetSpellProto()->AttributesEx & SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY)
    {
        Unit::AuraMap& Auras = m_target->GetAuras();
        for (Unit::AuraMap::iterator iter = Auras.begin(); iter != Auras.end();)
        {
            SpellEntry const *spell = iter->second->GetSpellProto();
            if (spell->Id != GetId())
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

    m_target->ApplySpellImmune(GetId(),IMMUNITY_MECHANIC,GetMiscValue(),apply);

    // Demonic Empowerment -- voidwalker -- missing movement impairing effects immunity
    if (GetId() == 54508)
    {
        if (apply)
            m_target->RemoveMovementImpairingAuras();

        m_target->ApplySpellImmune(GetId(),IMMUNITY_STATE,SPELL_AURA_MOD_ROOT,apply);
        m_target->ApplySpellImmune(GetId(),IMMUNITY_STATE,SPELL_AURA_MOD_DECREASE_SPEED,apply);
    }
}

void AuraEffect::HandleAuraModEffectImmunity(bool apply, bool Real, bool /*changeAmount*/)
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

void AuraEffect::HandleAuraModStateImmunity(bool apply, bool Real, bool /*changeAmount*/)
{
    if(apply && Real && GetSpellProto()->AttributesEx & SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY)
    {
        m_target->RemoveAurasByType(AuraType(GetMiscValue()), NULL , GetParentAura());
    }

    m_target->ApplySpellImmune(GetId(),IMMUNITY_STATE,GetMiscValue(),apply);
}

void AuraEffect::HandleAuraModSchoolImmunity(bool apply, bool Real, bool /*changeAmount*/)
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
        for (Unit::AuraMap::iterator iter = Auras.begin(); iter != Auras.end();)
        {
            SpellEntry const *spell = iter->second->GetSpellProto();
            if((GetSpellSchoolMask(spell) & school_mask)//Check for school mask
                && IsDispelableBySpell(GetSpellProto(),spell->Id, true)
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

void AuraEffect::HandleAuraModDmgImmunity(bool apply, bool Real, bool /*changeAmount*/)
{
    m_target->ApplySpellImmune(GetId(),IMMUNITY_DAMAGE,GetMiscValue(),apply);
}

void AuraEffect::HandleAuraModDispelImmunity(bool apply, bool Real, bool /*changeAmount*/)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    m_target->ApplySpellDispelImmunity(m_spellProto, DispelType(GetMiscValue()), apply);
}

void AuraEffect::HandleAuraProcTriggerSpell(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;
    // Elemental oath - "while Clearcasting from Elemental Focus is active, you deal 5%/10% more spell damage."
    if (m_target->GetTypeId() == TYPEID_PLAYER && (GetId() == 51466 || GetId() == 51470))
    {
        if (apply)
        {
            SpellModifier *mod = new SpellModifier;
            mod->op = SPELLMOD_EFFECT2;
            mod->value = m_target->CalculateSpellDamage(GetSpellProto(), 1, GetSpellProto()->EffectBasePoints[1], m_target);
            mod->type = SPELLMOD_FLAT;
            mod->spellId = GetId();
            mod->mask[1] = 0x0004000;
            m_spellmod = mod;
        }
        ((Player*)m_target)->AddSpellMod(m_spellmod, apply);
    }
}

void AuraEffect::HandleAuraModStalked(bool apply, bool Real, bool /*changeAmount*/)
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

void AuraEffect::HandlePeriodicTriggerSpell(bool apply, bool Real, bool /*changeAmount*/)
{
    m_isPeriodic = apply;
    if (!apply) 
    { 
        switch(m_spellProto->Id) 
        { 
            case 66:                                        // Invisibility 
                    m_target->CastSpell(m_target, 32612, true, NULL, this); 

                return; 
            case 42783:                                     //Wrath of the Astrom... 
                    m_target->CastSpell(m_target, m_spellProto->CalculateSimpleValue(GetEffIndex()+1), true); 
                return; 
            default: 
                break; 
        } 
    }
}

void AuraEffect::HandlePeriodicTriggerSpellWithValue(bool apply, bool Real, bool /*changeAmount*/)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandlePeriodicEnergize(bool apply, bool Real, bool changeAmount)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandleAuraPowerBurn(bool apply, bool Real, bool /*changeAmount*/)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandleAuraPeriodicDummy(bool apply, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;

    Unit* caster = GetCaster();

    SpellEntry const*spell = GetSpellProto();
    switch( spell->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            if(spell->Id == 62399) // Overload Circuit
            {
                if(m_target->GetMap()->IsDungeon())
                    if(m_target->GetAuras().count(62399) >= (((InstanceMap*)m_target->GetMap())->GetDifficulty() == REGULAR_DIFFICULTY ? 2 : 4))
                    {
                        m_target->CastSpell(m_target, 62475, true); // System Shutdown
                        if(Unit *veh = m_target->GetVehicleBase())
                            veh->CastSpell(m_target, 62475, true);
                    }
            }
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            switch (spell->Id)
            {
                // Demonic Circle
                case 48018:
                    if (!apply)
                    {
                        // Do not remove GO when aura is removed by stack
                        // to prevent remove GO added by new spell
                        // old one is already removed
                        if (GetParentAura()->GetRemoveMode()!=AURA_REMOVE_BY_STACK)
                            m_target->RemoveGameObject(spell->Id,true);
                        m_target->RemoveAura(62388);
                    }
                break;
            }
        }
        case SPELLFAMILY_DEATHKNIGHT:
        {
            // Reaping
            // Blood of the North
            // Death Rune Mastery
            if (spell->SpellIconID == 3041 || spell->SpellIconID == 22 || spell->SpellIconID == 2622)
                m_amount = 0;
            break;
        }
    }

    m_isPeriodic = apply;
}

void AuraEffect::HandlePeriodicHeal(bool apply, bool Real, bool /*changeAmount*/)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandlePeriodicDamage(bool apply, bool Real, bool changeAmount)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandlePeriodicDamagePCT(bool apply, bool Real, bool /*changeAmount*/)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandlePeriodicLeech(bool apply, bool Real, bool /*changeAmount*/)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandlePeriodicManaLeech(bool apply, bool Real, bool /*changeAmount*/)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandlePeriodicHealthFunnel(bool apply, bool Real, bool /*changeAmount*/)
{
    m_isPeriodic = apply;
}

/*********************************************************/
/***                  MODIFY STATS                     ***/
/*********************************************************/

/********************************/
/***        RESISTANCE        ***/
/********************************/

void AuraEffect::HandleAuraModResistanceExclusive(bool apply, bool Rea, bool /*changeAmount*/)
{
    for (int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL; x++)
    {
        if(GetMiscValue() & int32(1<<x))
        {
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_VALUE, float(m_amount), apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER)
                m_target->ApplyResistanceBuffModsMod(SpellSchools(x),GetParentAura()->IsPositive(),m_amount, apply);
        }
    }
}

void AuraEffect::HandleAuraModResistance(bool apply, bool Real, bool /*changeAmount*/)
{
    for (int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL; x++)
    {
        if(GetMiscValue() & int32(1<<x))
        {
            m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), TOTAL_VALUE, float(m_amount), apply);
            if(m_target->GetTypeId() == TYPEID_PLAYER || ((Creature*)m_target)->isPet())
                m_target->ApplyResistanceBuffModsMod(SpellSchools(x),m_amount > 0,m_amount, apply);
        }
    }
}

void AuraEffect::HandleAuraModBaseResistancePCT(bool apply, bool Real, bool /*changeAmount*/)
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
        for (int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL; x++)
        {
            if(GetMiscValue() & int32(1<<x))
                m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_PCT, float(m_amount), apply);
        }
    }
}

void AuraEffect::HandleModResistancePercent(bool apply, bool Real, bool /*changeAmount*/)
{
    for (int8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
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

void AuraEffect::HandleModBaseResistance(bool apply, bool Real, bool /*changeAmount*/)
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
        for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
            if(GetMiscValue() & (1<<i))
                m_target->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_VALUE, float(m_amount), apply);
    }
}

/********************************/
/***           STAT           ***/
/********************************/

void AuraEffect::HandleAuraModStat(bool apply, bool Real, bool /*changeAmount*/)
{
    if (GetMiscValue() < -2 || GetMiscValue() > 4)
    {
        sLog.outError("WARNING: Spell %u effect %u have unsupported misc value (%i) for SPELL_AURA_MOD_STAT ",GetId(),GetEffIndex(),GetMiscValue());
        return;
    }

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; i++)
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

void AuraEffect::HandleModPercentStat(bool apply, bool Real, bool /*changeAmount*/)
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

void AuraEffect::HandleModSpellDamagePercentFromStat(bool /*apply*/, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Magic damage modifiers implemented in Unit::SpellDamageBonus
    // This information for client side use only
    // Recalculate bonus
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModSpellHealingPercentFromStat(bool apply, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate bonus
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModSpellDamagePercentFromAttackPower(bool /*apply*/, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Magic damage modifiers implemented in Unit::SpellDamageBonus
    // This information for client side use only
    // Recalculate bonus
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModSpellHealingPercentFromAttackPower(bool /*apply*/, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate bonus
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModHealingDone(bool /*apply*/, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    // implemented in Unit::SpellHealingBonus
    // this information is for client side only
    ((Player*)m_target)->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModTotalPercentStat(bool apply, bool Real, bool /*changeAmount*/)
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

void AuraEffect::HandleAuraModResistenceOfStatPercent(bool /*apply*/, bool Real, bool /*changeAmount*/)
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
void AuraEffect::HandleAuraModTotalHealthPercentRegen(bool apply, bool Real, bool /*changeAmount*/)
{
    m_isPeriodic = apply;
}

void AuraEffect::HandleAuraModTotalEnergyPercentRegen(bool apply, bool Real, bool /*changeAmount*/)
{
    if(m_amplitude == 0)
        m_amplitude = 1000;

    m_periodicTimer = m_amplitude;
    m_isPeriodic = apply;
}

void AuraEffect::HandleModRegen(bool apply, bool Real, bool /*changeAmount*/)            // eating
{
    if(m_amplitude == 0)
        m_amplitude = 5000;

    m_periodicTimer = 5000;
    m_isPeriodic = apply;
}

void AuraEffect::HandleModPowerRegen(bool apply, bool Real, bool changeAmount)       // drinking
{
    if(!Real && !changeAmount)
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

void AuraEffect::HandleModPowerRegenPCT(bool /*apply*/, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;

    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Update manaregen value
    if (GetMiscValue() == POWER_MANA)
        ((Player*)m_target)->UpdateManaRegen();
}

void AuraEffect::HandleModManaRegen(bool /*apply*/, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;

    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    //Note: an increase in regen does NOT cause threat.
    ((Player*)m_target)->UpdateManaRegen();
}

void AuraEffect::HandleComprehendLanguage(bool apply, bool Real, bool /*changeAmount*/)
{
    if(apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
}

void AuraEffect::HandleAuraModIncreaseHealth(bool apply, bool Real, bool changeAmount)
{
    if(Real || changeAmount)
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

void AuraEffect::HandleAuraModIncreaseMaxHealth(bool apply, bool Real, bool /*changeAmount*/)
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

void AuraEffect::HandleAuraModIncreaseEnergy(bool apply, bool Real, bool /*changeAmount*/)
{
    Powers powerType = m_target->getPowerType();
    if(int32(powerType) != GetMiscValue())
        return;

    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + powerType);

    // Special case with temporary increase max/current power (percent)
    if (GetId()==64904)                                     // Hymn of Hope
    {
        if(Real)
        {
            uint32 val = m_target->GetPower(powerType);
            m_target->HandleStatModifier(unitMod, TOTAL_PCT, float(m_amount), apply);
            m_target->SetPower(powerType, apply ? val*(100+m_amount)/100 : val*100/(100+m_amount));
        }
        return;
    }

    // generic flat case
    m_target->HandleStatModifier(unitMod, TOTAL_VALUE, float(m_amount), apply);
}

void AuraEffect::HandleAuraModIncreaseEnergyPercent(bool apply, bool /*Real*/, bool /*changeAmount*/)
{
    Powers powerType = m_target->getPowerType();
    if(int32(powerType) != GetMiscValue())
        return;

    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + powerType);

    m_target->HandleStatModifier(unitMod, TOTAL_PCT, float(m_amount), apply);
}

void AuraEffect::HandleAuraModIncreaseHealthPercent(bool apply, bool /*Real*/, bool /*changeAmount*/)
{
    // Unit will keep hp% after MaxHealth being modified if unit is alive.
    float percent = ((float)m_target->GetHealth()) / m_target->GetMaxHealth();
    m_target->HandleStatModifier(UNIT_MOD_HEALTH, TOTAL_PCT, float(m_amount), apply);
    if (m_target->isAlive())
        m_target->SetHealth(uint32(m_target->GetMaxHealth()*percent));
}

void AuraEffect::HandleAuraIncreaseBaseHealthPercent(bool apply, bool /*Real*/, bool /*changeAmount*/)
{
    m_target->HandleStatModifier(UNIT_MOD_HEALTH, BASE_PCT, float(m_amount), apply);
}

/********************************/
/***          FIGHT           ***/
/********************************/

void AuraEffect::HandleAuraModParryPercent(bool /*apply*/, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)m_target)->UpdateParryPercentage();
}

void AuraEffect::HandleAuraModDodgePercent(bool /*apply*/, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)m_target)->UpdateDodgePercentage();
    //sLog.outError("BONUS DODGE CHANCE: + %f", float(m_amount));
}

void AuraEffect::HandleAuraModBlockPercent(bool /*apply*/, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)m_target)->UpdateBlockPercentage();
    //sLog.outError("BONUS BLOCK CHANCE: + %f", float(m_amount));
}

void AuraEffect::HandleAuraModRegenInterrupt(bool /*apply*/, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)m_target)->UpdateManaRegen();
}

void AuraEffect::HandleAuraModWeaponCritPercent(bool apply, bool Real, bool changeAmount)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // apply item specific bonuses for already equipped weapon
    if(Real || changeAmount)
    {
        for (int i = 0; i < MAX_ATTACK; ++i)
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

void AuraEffect::HandleModHitChance(bool apply, bool Real, bool /*changeAmount*/)
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

void AuraEffect::HandleModSpellHitChance(bool apply, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_target)->UpdateSpellHitChances();
    else
        m_target->m_modSpellHitChance += apply ? m_amount: (-m_amount);
}

void AuraEffect::HandleModSpellCritChance(bool apply, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;

    if(m_target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_target)->UpdateAllSpellCritChances();
    else
        m_target->m_baseSpellCritChance += apply ? m_amount:-m_amount;
}

void AuraEffect::HandleModSpellCritChanceShool(bool /*apply*/, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    for (int school = SPELL_SCHOOL_NORMAL; school < MAX_SPELL_SCHOOL; ++school)
        if (GetMiscValue() & (1<<school))
            ((Player*)m_target)->UpdateSpellCritChance(school);
}

/********************************/
/***         ATTACK SPEED     ***/
/********************************/

void AuraEffect::HandleModCastingSpeed(bool apply, bool Real, bool /*changeAmount*/)
{
    m_target->ApplyCastTimePercentMod(m_amount,apply);
}

void AuraEffect::HandleModMeleeRangedSpeedPct(bool apply, bool Real, bool /*changeAmount*/)
{
    m_target->ApplyAttackTimePercentMod(BASE_ATTACK,m_amount,apply);
    m_target->ApplyAttackTimePercentMod(OFF_ATTACK,m_amount,apply);
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK, m_amount, apply);
}

void AuraEffect::HandleModCombatSpeedPct(bool apply, bool Real, bool /*changeAmount*/)
{
    m_target->ApplyCastTimePercentMod(m_amount,apply);
    m_target->ApplyAttackTimePercentMod(BASE_ATTACK,m_amount,apply);
    m_target->ApplyAttackTimePercentMod(OFF_ATTACK,m_amount,apply);
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK, m_amount, apply);
}

void AuraEffect::HandleModAttackSpeed(bool apply, bool Real, bool /*changeAmount*/)
{
    m_target->ApplyAttackTimePercentMod(BASE_ATTACK,m_amount,apply);
    m_target->UpdateDamagePhysical(BASE_ATTACK);
}

void AuraEffect::HandleHaste(bool apply, bool Real, bool /*changeAmount*/)
{
    m_target->ApplyAttackTimePercentMod(BASE_ATTACK,  m_amount,apply);
    m_target->ApplyAttackTimePercentMod(OFF_ATTACK,   m_amount,apply);
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK,m_amount,apply);
}

void AuraEffect::HandleAuraModRangedHaste(bool apply, bool Real, bool /*changeAmount*/)
{
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK, m_amount, apply);
}

void AuraEffect::HandleRangedAmmoHaste(bool apply, bool Real, bool /*changeAmount*/)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK,m_amount, apply);
}

/********************************/
/***        ATTACK POWER      ***/
/********************************/

void AuraEffect::HandleAuraModAttackPower(bool apply, bool Real, bool /*changeAmount*/)
{
    m_target->HandleStatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, float(m_amount), apply);
}

void AuraEffect::HandleAuraModRangedAttackPower(bool apply, bool Real, bool /*changeAmount*/)
{
    if((m_target->getClassMask() & CLASSMASK_WAND_USERS)!=0)
        return;

    m_target->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, float(m_amount), apply);
}

void AuraEffect::HandleAuraModAttackPowerPercent(bool apply, bool Real, bool /*changeAmount*/)
{
    //UNIT_FIELD_ATTACK_POWER_MULTIPLIER = multiplier - 1
    m_target->HandleStatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_PCT, float(m_amount), apply);
}

void AuraEffect::HandleAuraModRangedAttackPowerPercent(bool apply, bool Real, bool /*changeAmount*/)
{
    if((m_target->getClassMask() & CLASSMASK_WAND_USERS)!=0)
        return;

    //UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER = multiplier - 1
    m_target->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_PCT, float(m_amount), apply);
}

void AuraEffect::HandleAuraModRangedAttackPowerOfStatPercent(bool apply, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;

    // Recalculate bonus
    if(m_target->GetTypeId() == TYPEID_PLAYER && !(m_target->getClassMask() & CLASSMASK_WAND_USERS))
        ((Player*)m_target)->UpdateAttackPowerAndDamage(true);
}

void AuraEffect::HandleAuraModAttackPowerOfStatPercent(bool apply, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;

    // Recalculate bonus
    if(m_target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_target)->UpdateAttackPowerAndDamage(false);
}

void AuraEffect::HandleAuraModAttackPowerOfArmor(bool /*apply*/, bool Real, bool /*changeAmount*/)
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
void AuraEffect::HandleModDamageDone(bool apply, bool Real, bool changeAmount)
{
    // apply item specific bonuses for already equipped weapon
    if((Real || changeAmount) && m_target->GetTypeId() == TYPEID_PLAYER)
    {
        for (int i = 0; i < MAX_ATTACK; ++i)
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
            if(m_amount > 0)
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
        if(m_amount > 0)
        {
            for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; i++)
            {
                if((GetMiscValue() & (1<<i)) != 0)
                    m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS+i,m_amount,apply);
            }
        }
        else
        {
            for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; i++)
            {
                if((GetMiscValue() & (1<<i)) != 0)
                    m_target->ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG+i,m_amount,apply);
            }
        }
        if(Guardian* pet = ((Player*)m_target)->GetGuardianPet())
            pet->UpdateAttackPowerAndDamage();
    }
}

void AuraEffect::HandleModDamagePercentDone(bool apply, bool Real, bool changeAmount)
{
    sLog.outDebug("AURA MOD DAMAGE type:%u negative:%u", GetMiscValue(), m_amount > 0);

    // apply item specific bonuses for already equipped weapon
    if((Real || changeAmount) && m_target->GetTypeId() == TYPEID_PLAYER)
    {
        for (int i = 0; i < MAX_ATTACK; ++i)
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
        for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            m_target->ApplyModSignedFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT+i,m_amount/100.0f,apply);
}

void AuraEffect::HandleModOffhandDamagePercent(bool apply, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;

    sLog.outDebug("AURA MOD OFFHAND DAMAGE");

    m_target->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT, float(m_amount), apply);
}

/********************************/
/***        POWER COST        ***/
/********************************/

void AuraEffect::HandleModPowerCostPCT(bool apply, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;

    float amount = m_amount /100.0f;
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if(GetMiscValue() & (1<<i))
            m_target->ApplyModSignedFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER+i,amount,apply);
}

void AuraEffect::HandleModPowerCost(bool apply, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;

    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if(GetMiscValue() & (1<<i))
            m_target->ApplyModInt32Value(UNIT_FIELD_POWER_COST_MODIFIER+i,m_amount,apply);
}

void AuraEffect::HandleNoReagentUseAura(bool Apply, bool Real, bool /*changeAmount*/)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    flag96 mask;
    Unit::AuraEffectList const& noReagent = m_target->GetAurasByType(SPELL_AURA_NO_REAGENT_USE);
        for (Unit::AuraEffectList::const_iterator i = noReagent.begin(); i !=  noReagent.end(); ++i)
            mask |= (*i)->m_spellProto->EffectSpellClassMask[(*i)->m_effIndex];

    m_target->SetUInt32Value(PLAYER_NO_REAGENT_COST_1  , mask[0]);
    m_target->SetUInt32Value(PLAYER_NO_REAGENT_COST_1+1, mask[1]);
    m_target->SetUInt32Value(PLAYER_NO_REAGENT_COST_1+2, mask[2]);
}

/*********************************************************/
/***                    OTHERS                         ***/
/*********************************************************/

void AuraEffect::HandleAuraAllowOnlyAbility(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;

    if(!apply && m_target->HasAuraType(SPELL_AURA_ALLOW_ONLY_ABILITY))
        return;

    if(m_target->GetTypeId() == TYPEID_PLAYER)
    {
        if (apply)
            m_target->SetFlag(PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY);
        else
            m_target->RemoveFlag(PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY);
    }
}

void AuraEffect::HandleAuraEmpathy(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real || m_target->GetTypeId() != TYPEID_UNIT)
        return;

    CreatureInfo const * ci = objmgr.GetCreatureTemplate(m_target->GetEntry());
    if(ci && ci->type == CREATURE_TYPE_BEAST)
        m_target->ApplyModUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_SPECIALINFO, apply);
}

void AuraEffect::HandleAuraUntrackable(bool apply, bool Real, bool /*changeAmount*/)
{
    if (!Real)
        return;
    if(apply)
        m_target->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_UNTRACKABLE);
    else
        m_target->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_UNTRACKABLE);
}

void AuraEffect::HandleAuraModPacify(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;

    if(apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
}

void AuraEffect::HandleAuraModPacifyAndSilence(bool apply, bool Real, bool changeAmount)
{
    // Vengeance of the Blue Flight
    if(m_spellProto->Id == 45839)
    {
        if(apply)
            m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        else
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }
    HandleAuraModPacify(apply,Real, changeAmount);
    HandleAuraModSilence(apply,Real, changeAmount);
}

void AuraEffect::HandleAuraGhost(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real || m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if(apply)
        m_target->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
    else
        m_target->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
}

void AuraEffect::HandleAuraAllowFlight(bool apply, bool Real, bool /*changeAmount*/)
{
    // all applied/removed only at real aura add/remove
    if(!Real)
        return;

    if(m_target->GetTypeId() == TYPEID_UNIT)
        m_target->SetFlying(apply);

    if(Player *plr = m_target->m_movedPlayer)
    {
    // allow fly
    WorldPacket data;
    if(apply)
        data.Initialize(SMSG_MOVE_SET_CAN_FLY, 12);
    else
        data.Initialize(SMSG_MOVE_UNSET_CAN_FLY, 12);
    data.append(m_target->GetPackGUID());
    data << uint32(0);                                      // unk
        plr->SendDirectMessage(&data);
    }

    //m_target->SendMessageToSet(&data, true);
}

void AuraEffect::HandleModRating(bool apply, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
        if (GetMiscValue() & (1 << rating))
            ((Player*)m_target)->ApplyRatingMod(CombatRating(rating), m_amount, apply);
}

void AuraEffect::HandleModRatingFromStat(bool apply, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    // Just recalculate ratings
    for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
        if (GetMiscValue() & (1 << rating))
            ((Player*)m_target)->ApplyRatingMod(CombatRating(rating), 0, apply);
}

void AuraEffect::HandleForceMoveForward(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real || m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    if(apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVE);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVE);
}

void AuraEffect::HandleAuraModExpertise(bool /*apply*/, bool Real, bool changeAmount)
{
    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)m_target)->UpdateExpertise(BASE_ATTACK);
    ((Player*)m_target)->UpdateExpertise(OFF_ATTACK);
}

void AuraEffect::HandleModTargetResistance(bool apply, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
        return;
    // applied to damage as HandleNoImmediateEffect in Unit::CalcAbsorbResist and Unit::CalcArmorReducedDamage

    // show armor penetration
    if (m_target->GetTypeId() == TYPEID_PLAYER && (GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
        m_target->ApplyModInt32Value(PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE,m_amount, apply);

    // show as spell penetration only full spell penetration bonuses (all resistances except armor and holy
    if (m_target->GetTypeId() == TYPEID_PLAYER && (GetMiscValue() & SPELL_SCHOOL_MASK_SPELL)==SPELL_SCHOOL_MASK_SPELL)
        m_target->ApplyModInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE,m_amount, apply);
}

void AuraEffect::HandleShieldBlockValue(bool apply, bool Real, bool /*changeAmount*/)
{
    BaseModType modType = FLAT_MOD;
    if(m_auraName == SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT)
        modType = PCT_MOD;

    if(m_target->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_target)->HandleBaseModValue(SHIELD_BLOCK_VALUE, modType, float(m_amount), apply);
}

void AuraEffect::HandleAuraRetainComboPoints(bool apply, bool Real, bool changeAmount)
{
    // spells required only Real aura add/remove
    if(!Real && !changeAmount)
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

void AuraEffect::HandleModUnattackable( bool Apply, bool Real , bool /*changeAmount*/)
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

void AuraEffect::HandleSpiritOfRedemption( bool apply, bool Real , bool /*changeAmount*/)
{
    // spells required only Real aura add/remove
    if(!Real)
        return;

    // prepare spirit state
    if(apply)
    {
        if(m_target->GetTypeId() == TYPEID_PLAYER)
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

            // Consecrate ticks can miss and will not show up in the combat log
            if (GetSpellProto()->Effect[GetEffIndex()] == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
                pCaster->SpellHitResult(m_target,GetSpellProto(),false) != SPELL_MISS_NONE)
                return;

            // Check for immune (not use charges)
            if (m_target->IsImmunedToDamage(GetSpellProto()))
                return;

            // some auras remove at specific health level or more
            if (m_auraName == SPELL_AURA_PERIODIC_DAMAGE)
            {
                switch (GetId())
                {
                    case 43093: case 31956: case 38801:  // Grievous Wound
                    case 35321: case 38363: case 39215:  // Gushing Wound
                        if(m_target->GetHealth() == m_target->GetMaxHealth())
                        {
                            m_target->RemoveAurasDueToSpell(GetId());
                            return;
                        }
                        break;
                    case 38772: // Grievous Wound
                    {
                        uint32 percent =
                            GetEffIndex() < 2 && GetSpellProto()->Effect[GetEffIndex()]==SPELL_EFFECT_DUMMY ?
                            pCaster->CalculateSpellDamage(GetSpellProto(),GetEffIndex()+1,GetSpellProto()->EffectBasePoints[GetEffIndex()+1],m_target) :
                            100;
                        if(m_target->GetHealth()*100 >= m_target->GetMaxHealth()*percent)
                        {
                            m_target->RemoveAurasDueToSpell(GetId());
                            return;
                        }
                        break;
                    }
                    case 41337: // Aura of Anger
                    {
                        if (AuraEffect * aurEff = GetParentAura()->GetPartAura(1))
                        {
                            aurEff->ApplyModifier(false, false, true);
                            aurEff->SetAmount(aurEff->GetAmount()+5);
                            aurEff->ApplyModifier(true, false, true);
                        }
                        m_amount = 100 * m_tickNumber;
                        break;
                    }
                    // Brutallus Burn
                    case 46394:
                    {
                        if (m_tickNumber % 11 == 0)
                            m_amount *= 2;
                        break;
                    }
                    case 31803:
                    case 53742:
                    {
                        float ap = pCaster->GetTotalAttackPowerValue(BASE_ATTACK);
                        int32 holy = pCaster->SpellBaseDamageBonus(GetSpellSchoolMask(m_spellProto)) +
                                     pCaster->SpellBaseDamageBonusForVictim(GetSpellSchoolMask(m_spellProto), m_target);

                        m_amount = (0.013f * holy + 0.025f * ap) * 6 / 5 * m_tickNumber;
                        break;
                    }
                    default:
                        break;
                }
            }

            uint32 absorb=0;
            uint32 resist=0;
            CleanDamage cleanDamage =  CleanDamage(0, 0, BASE_ATTACK, MELEE_HIT_NORMAL );

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
                    cleanDamage.mitigated_damage += pdamage - pdamageReductedArmor;
                    pdamage = pdamageReductedArmor;
                }

                // Curse of Agony damage-per-tick calculation
                if (GetSpellProto()->SpellFamilyName==SPELLFAMILY_WARLOCK && (GetSpellProto()->SpellFamilyFlags[0] & 0x400) && GetSpellProto()->SpellIconID==544)
                {
                    uint32 totalTick = GetParentAura()->GetAuraMaxDuration() / m_amplitude;
                    // 1..4 ticks, 1/2 from normal tick damage
                    if(m_tickNumber <= totalTick / 3)
                        pdamage = pdamage/2;
                    // 9..12 ticks, 3/2 from normal tick damage
                    else if(m_tickNumber > totalTick * 2 / 3)
                        pdamage += (pdamage+1)/2;           // +1 prevent 0.5 damage possible lost at 1..4 ticks
                    // 5..8 ticks have normal tick damage
                }
                // There is a Chance to make a Soul Shard when Drain soul does damage
                if (GetSpellProto()->SpellFamilyName==SPELLFAMILY_WARLOCK && (GetSpellProto()->SpellFamilyFlags[0] & 0x00004000))
                {
                    if(roll_chance_i(20))
                        pCaster->CastSpell(pCaster, 24827, true, 0, this);
                }
            }
            else
                pdamage = uint32(m_target->GetMaxHealth()*pdamage/100);

            bool crit = IsPeriodicTickCrit(pCaster);
            if (crit)
                pdamage = pCaster->SpellCriticalDamageBonus(m_spellProto, pdamage, m_target);

            // only from players
            if (IS_PLAYER_GUID(GetCasterGUID()))
                pdamage -= m_target->GetSpellDamageReduction(pdamage);

            pCaster->CalcAbsorbResist(m_target, GetSpellSchoolMask(GetSpellProto()), DOT, pdamage, &absorb, &resist, m_spellProto);

            sLog.outDetail("PeriodicTick: %u (TypeId: %u) attacked %u (TypeId: %u) for %u dmg inflicted by %u abs is %u",
                GUID_LOPART(GetCasterGUID()), GuidHigh2TypeId(GUID_HIPART(GetCasterGUID())), m_target->GetGUIDLow(), m_target->GetTypeId(), pdamage, GetId(),absorb);

            pCaster->DealDamageMods(m_target,pdamage,&absorb);

            Unit* target = m_target;                        // aura can be deleted in DealDamage
            SpellEntry const* spellProto = GetSpellProto();

            // Set trigger flag
            uint32 procAttacker = PROC_FLAG_ON_DO_PERIODIC;
            uint32 procVictim   = PROC_FLAG_ON_TAKE_PERIODIC;
            uint32 procEx = PROC_EX_NORMAL_HIT | PROC_EX_INTERNAL_DOT;
            pdamage = (pdamage <= absorb+resist) ? 0 : (pdamage-absorb-resist);
            if (pdamage)
                procVictim|=PROC_FLAG_TAKEN_ANY_DAMAGE;

            int32 overkill = pdamage - m_target->GetHealth();
            if (overkill < 0)
              overkill = 0;

            SpellPeriodicAuraLogInfo pInfo(this, pdamage, overkill, absorb, resist, 0.0f, crit);
            m_target->SendPeriodicAuraLog(&pInfo);

            pCaster->ProcDamageAndSpell(target, procAttacker, procVictim, procEx, pdamage, BASE_ATTACK, spellProto);

            pCaster->DealDamage(target, pdamage, &cleanDamage, DOT, GetSpellSchoolMask(spellProto), spellProto, true);
            break;
        }
        case SPELL_AURA_PERIODIC_LEECH:
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
            if(m_target->IsImmunedToDamage(GetSpellProto()))
                return;

            uint32 absorb=0;
            uint32 resist=0;
            CleanDamage cleanDamage =  CleanDamage(0, 0, BASE_ATTACK, MELEE_HIT_NORMAL );

            //uint32 pdamage = GetModifierValuePerStack() > 0 ? GetModifierValuePerStack() : 0;
            uint32 pdamage = GetAmount() > 0 ? GetAmount() : 0;
            pdamage = pCaster->SpellDamageBonus(m_target, GetSpellProto(), pdamage, DOT, GetParentAura()->GetStackAmount());

            bool crit = IsPeriodicTickCrit(pCaster);
            if (crit)
                pdamage = pCaster->SpellCriticalDamageBonus(m_spellProto, pdamage, m_target);

            //Calculate armor mitigation if it is a physical spell
            if (GetSpellSchoolMask(GetSpellProto()) & SPELL_SCHOOL_MASK_NORMAL)
            {
                uint32 pdamageReductedArmor = pCaster->CalcArmorReducedDamage(m_target, pdamage, GetSpellProto());
                cleanDamage.mitigated_damage += pdamage - pdamageReductedArmor;
                pdamage = pdamageReductedArmor;
            }

            // Reduce dot damage from resilience for players.
            if (m_target->GetTypeId() == TYPEID_PLAYER)
                pdamage-=((Player*)m_target)->GetSpellDamageReduction(pdamage);

            pCaster->CalcAbsorbResist(m_target, GetSpellSchoolMask(GetSpellProto()), DOT, pdamage, &absorb, &resist, m_spellProto);

            if(m_target->GetHealth() < pdamage)
                pdamage = uint32(m_target->GetHealth());

            sLog.outDetail("PeriodicTick: %u (TypeId: %u) health leech of %u (TypeId: %u) for %u dmg inflicted by %u abs is %u",
                GUID_LOPART(GetCasterGUID()), GuidHigh2TypeId(GUID_HIPART(GetCasterGUID())), m_target->GetGUIDLow(), m_target->GetTypeId(), pdamage, GetId(),absorb);

            pCaster->SendSpellNonMeleeDamageLog(m_target, GetId(), pdamage, GetSpellSchoolMask(GetSpellProto()), absorb, resist, false, 0, crit);

            Unit* target = m_target;                        // aura can be deleted in DealDamage
            SpellEntry const* spellProto = GetSpellProto();
            float multiplier = spellProto->EffectMultipleValue[GetEffIndex()] > 0 ? spellProto->EffectMultipleValue[GetEffIndex()] : 1;
            int32 stackAmount = GetParentAura()->GetStackAmount();

            // Set trigger flag
            uint32 procAttacker = PROC_FLAG_ON_DO_PERIODIC;
            uint32 procVictim   = PROC_FLAG_ON_TAKE_PERIODIC;
            uint32 procEx = PROC_EX_NORMAL_HIT | PROC_EX_INTERNAL_DOT;
            pdamage = (pdamage <= absorb+resist) ? 0 : (pdamage-absorb-resist);
            if (pdamage)
                procVictim|=PROC_FLAG_TAKEN_ANY_DAMAGE;
            pCaster->ProcDamageAndSpell(target, procAttacker, procVictim, procEx, pdamage, BASE_ATTACK, spellProto);
            int32 new_damage = pCaster->DealDamage(target, pdamage, &cleanDamage, DOT, GetSpellSchoolMask(spellProto), spellProto, false);

            if (!target->isAlive() && pCaster->IsNonMeleeSpellCasted(false))
                for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
                    if (Spell* spell = pCaster->GetCurrentSpell(CurrentSpellTypes(i)))
                        if (spell->m_spellInfo->Id == GetId())
                            spell->cancel();

            if(Player *modOwner = pCaster->GetSpellModOwner())
                modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_MULTIPLE_VALUE, multiplier);

            uint32 heal = uint32(pCaster->SpellHealingBonus(pCaster, spellProto, uint32(new_damage * multiplier), DOT, stackAmount));

            int32 gain = pCaster->DealHeal(pCaster, heal, spellProto);
            pCaster->getHostilRefManager().threatAssist(pCaster, gain * 0.5f, spellProto);
            break;
        }
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL: // only three spells
        {
            Unit *donator = GetCaster();
            if(!donator || !donator->GetHealth())
                return;

            uint32 pdamage = GetAmount() * GetParentAura()->GetStackAmount();
            if(donator->GetHealth() < pdamage)
                pdamage = donator->GetHealth() - 1;
            if(!pdamage)
                return;

            Unit* target = m_target;                        // aura can be deleted in DealDamage
            SpellEntry const* spellProto = GetSpellProto();
            //donator->SendSpellNonMeleeDamageLog(donator, GetId(), pdamage, GetSpellSchoolMask(spellProto), 0, 0, false, 0);
            donator->ModifyHealth(-(int32)pdamage);
            sLog.outDetail("PeriodicTick: donator %u target %u damage %u.", donator->GetEntry(), target->GetEntry(), pdamage);

            if(spellProto->EffectMultipleValue[GetEffIndex()] > 0)
                pdamage *= spellProto->EffectMultipleValue[GetEffIndex()];

            donator->DealHeal(target, pdamage, spellProto);
            break;
        }
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_OBS_MOD_HEALTH:
        {
            Unit *pCaster = GetCaster();
            if(!pCaster)
                return;

            // heal for caster damage (must be alive)
            if(m_target != pCaster && GetSpellProto()->AttributesEx2 & SPELL_ATTR_EX2_HEALTH_FUNNEL && !pCaster->isAlive())
                return;

            if(GetParentAura()->GetAuraDuration() ==-1 && m_target->GetHealth()==m_target->GetMaxHealth())
                return;

            // ignore non positive values (can be result apply spellmods to aura damage
            //uint32 amount = GetModifierValuePerStack() > 0 ? GetModifierValuePerStack() : 0;
            int32 pdamage = GetAmount() > 0 ? GetAmount() : 0;

            if(m_auraName==SPELL_AURA_OBS_MOD_HEALTH)
                pdamage = uint32(m_target->GetMaxHealth() * pdamage * GetParentAura()->GetStackAmount() / 100);
            else
            {
                // Wild Growth (1/7 - 6 + 2*ramainTicks) %
                if (m_spellProto->SpellFamilyName == SPELLFAMILY_DRUID && m_spellProto->SpellIconID == 2864)
                {
                    int32 ticks = GetParentAura()->GetAuraMaxDuration()/m_amplitude;
                    int32 remainingTicks = int32(float(GetParentAura()->GetAuraDuration()) / m_amplitude + 0.5);
                    pdamage = int32(pdamage) + int32(pdamage)*ticks*(-6+2*remainingTicks)/100;
                }

                pdamage = pCaster->SpellHealingBonus(m_target, GetSpellProto(), pdamage, DOT, GetParentAura()->GetStackAmount());
            }

            bool crit = IsPeriodicTickCrit(pCaster);
            if (crit)
                pdamage = pCaster->SpellCriticalHealingBonus(m_spellProto, pdamage, m_target);

            sLog.outDetail("PeriodicTick: %u (TypeId: %u) heal of %u (TypeId: %u) for %u health inflicted by %u",
                GUID_LOPART(GetCasterGUID()), GuidHigh2TypeId(GUID_HIPART(GetCasterGUID())), m_target->GetGUIDLow(), m_target->GetTypeId(), pdamage, GetId());

            int32 gain = m_target->ModifyHealth(pdamage);

            SpellPeriodicAuraLogInfo pInfo(this, pdamage, pdamage - gain, 0, 0, 0.0f, crit);
            m_target->SendPeriodicAuraLog(&pInfo);

            // add HoTs to amount healed in bgs
            if( pCaster->GetTypeId() == TYPEID_PLAYER )
                if( BattleGround *bg = ((Player*)pCaster)->GetBattleGround() )
                    bg->UpdatePlayerScore(((Player*)pCaster), SCORE_HEALING_DONE, gain);

            m_target->getHostilRefManager().threatAssist(pCaster, float(gain) * 0.5f, GetSpellProto());

            Unit* target = m_target;                        // aura can be deleted in DealDamage
            SpellEntry const* spellProto = GetSpellProto();
            bool haveCastItem = GetParentAura()->GetCastItemGUID()!=0;

            // Health Funnel
            // heal for caster damage
            if(m_target!=pCaster && GetSpellProto()->AttributesEx2 & SPELL_ATTR_EX2_HEALTH_FUNNEL)
            {
                uint32 dmg = spellProto->manaPerSecond;
                if(pCaster->GetHealth() <= dmg && pCaster->GetTypeId() == TYPEID_PLAYER)
                {
                    pCaster->RemoveAurasDueToSpell(GetId());

                    // finish current generic/channeling spells, don't affect autorepeat
                    pCaster->FinishSpell(CURRENT_GENERIC_SPELL);
                    pCaster->FinishSpell(CURRENT_CHANNELED_SPELL);
                }
                else
                {
                    uint32 damage = gain;
                    uint32 absorb = 0;
                    pCaster->DealDamageMods(pCaster,damage,&absorb);
                    pCaster->SendSpellNonMeleeDamageLog(pCaster, GetId(), damage, GetSpellSchoolMask(GetSpellProto()), absorb, 0, false, 0, false);

                    CleanDamage cleanDamage =  CleanDamage(0, 0, BASE_ATTACK, MELEE_HIT_NORMAL );
                    pCaster->DealDamage(pCaster, damage, &cleanDamage, NODAMAGE, GetSpellSchoolMask(GetSpellProto()), GetSpellProto(), true);
                }
            }

            uint32 procAttacker = PROC_FLAG_ON_DO_PERIODIC;
            uint32 procVictim   = PROC_FLAG_ON_TAKE_PERIODIC;
            uint32 procEx = PROC_EX_NORMAL_HIT | PROC_EX_INTERNAL_HOT;
            // ignore item heals
            if(!haveCastItem)
                pCaster->ProcDamageAndSpell(target, procAttacker, procVictim, procEx, pdamage, BASE_ATTACK, spellProto);
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
            if(m_target->IsImmunedToDamage(GetSpellProto()))
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
            if (power == POWER_MANA)
                drain_amount -= m_target->GetSpellCritDamageReduction(drain_amount);

            m_target->ModifyPower(power, -drain_amount);

            float gain_multiplier = 0;

            if(pCaster->GetMaxPower(power) > 0)
            {
                gain_multiplier = GetSpellProto()->EffectMultipleValue[GetEffIndex()];

                if(Player *modOwner = pCaster->GetSpellModOwner())
                    modOwner->ApplySpellMod(GetId(), SPELLMOD_MULTIPLE_VALUE, gain_multiplier);
            }

            SpellPeriodicAuraLogInfo pInfo(this, drain_amount, 0, 0, 0, gain_multiplier, false);
            m_target->SendPeriodicAuraLog(&pInfo);

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
            // Mana Feed - Drain Mana
            if (m_spellProto->SpellFamilyName == SPELLFAMILY_WARLOCK
                && m_spellProto->SpellFamilyFlags[0] & 0x00000010)
            {
                int32 manaFeedVal = 0;
                if (AuraEffect const * aurEff = GetParentAura()->GetPartAura(1))
                    manaFeedVal = aurEff->GetAmount();

                if(manaFeedVal > 0)
                {
                    manaFeedVal = manaFeedVal * gain_amount / 100;
                    pCaster->CastCustomSpell(pCaster, 32554, &manaFeedVal, NULL, NULL, true, NULL, this);
                }
            }
            break;
        }
        case SPELL_AURA_OBS_MOD_POWER:
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

            SpellPeriodicAuraLogInfo pInfo(this, amount, 0, 0, 0, 0.0f, false);
            m_target->SendPeriodicAuraLog(&pInfo);

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

            SpellPeriodicAuraLogInfo pInfo(this, amount, 0, 0, 0, 0.0f, false);
            m_target->SendPeriodicAuraLog(&pInfo);

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
            if(m_target->IsImmunedToDamage(GetSpellProto()))
                return;

            int32 pdamage = m_amount > 0 ? m_amount : 0;

            Powers powerType = Powers(GetMiscValue());

            if(!m_target->isAlive() || m_target->getPowerType() != powerType)
                return;

            // resilience reduce mana draining effect at spell crit damage reduction (added in 2.4)
            if (powerType == POWER_MANA)
                pdamage -= m_target->GetSpellCritDamageReduction(pdamage);

            uint32 gain = uint32(-m_target->ModifyPower(powerType, -pdamage));

            gain = uint32(gain * GetSpellProto()->EffectMultipleValue[GetEffIndex()]);

            SpellEntry const* spellProto = GetSpellProto();
            //maybe has to be sent different to client, but not by SMSG_PERIODICAURALOG
            SpellNonMeleeDamage damageInfo(pCaster, m_target, spellProto->Id, spellProto->SchoolMask);
            //no SpellDamageBonus for burn mana
            pCaster->CalculateSpellDamageTaken(&damageInfo, gain, spellProto);

            pCaster->DealDamageMods(damageInfo.target,damageInfo.damage,&damageInfo.absorb);

            pCaster->SendSpellNonMeleeDamageLog(&damageInfo);

            // Set trigger flag
            uint32 procAttacker = PROC_FLAG_ON_DO_PERIODIC;
            uint32 procVictim   = PROC_FLAG_ON_TAKE_PERIODIC;
            uint32 procEx       = createProcExtendMask(&damageInfo, SPELL_MISS_NONE) | PROC_EX_INTERNAL_DOT;
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
            // Butchery
            else if (m_spellProto->SpellFamilyName==SPELLFAMILY_DEATHKNIGHT
                && m_spellProto->SpellIconID==2664)
            {
                if (m_target->isInCombat())
                    m_target->ModifyPower(pt,m_amount);
            }

            // Anger Management
            // amount = 1+ 16 = 17 = 3,4*5 = 10,2*5/3
            // so 17 is rounded amount for 5 sec tick grow ~ 1 range grow in 3 sec
            if(pt == POWER_RAGE)
                m_target->ModifyPower(pt, m_amount*3/5);
            break;
        }
        case SPELL_AURA_DUMMY:
        {
            // Haunting Spirits
            if (GetId() == 7057)
            {
                m_target->CastSpell((Unit*)NULL , m_amount , true);
                m_amplitude = irand (0 , 60 ) + 30;
                m_amplitude *= IN_MILISECONDS;
                break;
            }
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
            Unit *pCaster = GetCaster();
            if (!pCaster)
                return;

            if (pCaster->GetTypeId() == TYPEID_UNIT && ((Creature*)pCaster)->isTotem() && ((Totem*)pCaster)->GetTotemType() != TOTEM_STATUE)
            {
                uint32 procAttacker = PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT;
                uint32 procVictim   = PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT;
                SpellEntry const *spellProto = GetSpellProto();

                if (spellProto->SpellFamilyName == SPELLFAMILY_GENERIC) // SPELLFAMILY_GENERIC proc by triggered spell
                {
                    uint32 trigger_spell_id = spellProto->EffectTriggerSpell[m_effIndex];
                    SpellEntry const *triggeredSpellInfo = sSpellStore.LookupEntry(trigger_spell_id);
                    ((Totem*)pCaster)->GetOwner()->ProcDamageAndSpell(pCaster, procAttacker, procVictim, PROC_EX_NORMAL_HIT, 0, BASE_ATTACK, triggeredSpellInfo);
                }
                else
                    ((Totem*)pCaster)->GetOwner()->ProcDamageAndSpell(pCaster, procAttacker, procVictim, PROC_EX_NORMAL_HIT, 0, BASE_ATTACK, spellProto);
            }

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
            case 43182:
            case 43183:
            case 46755:
            case 49472: // Drink Coffee
            case 57073:
            case 61830:
            {
                if (m_target->GetTypeId() != TYPEID_PLAYER)
                    return;
                // Get SPELL_AURA_MOD_POWER_REGEN aura from spell
                if (AuraEffect * aurEff = GetParentAura()->GetPartAura(0))
                {
                    if (aurEff->GetAuraName() !=SPELL_AURA_MOD_POWER_REGEN)
                    {
                        m_isPeriodic = false;
                        sLog.outError("Aura %d structure has been changed - first aura is no longer SPELL_AURA_MOD_POWER_REGEN", spell->Id);
                    }
                    else
                    {
                        // default case - not in arena
                        if (!((Player*)m_target)->InArena())
                        {
                            aurEff->SetAmount(GetAmount());
                            ((Player*)m_target)->UpdateManaRegen();
                            m_isPeriodic = false;
                        }
                        else
                        {
                            //**********************************************
                            // This feature uses only in arenas
                            //**********************************************
                            // Here need increase mana regen per tick (6 second rule)
                            // on 0 tick -   0  (handled in 2 second)
                            // on 1 tick - 166% (handled in 4 second)
                            // on 2 tick - 133% (handled in 6 second)

                            // Apply bonus for 1 - 4 tick
                            switch (m_tickNumber)
                            {
                                case 1:   // 0%
                                    aurEff->SetAmount(0);
                                    break;
                                case 2:   // 166%
                                    aurEff->SetAmount(GetAmount() * 5 / 3);
                                    break;
                                case 3:   // 133%
                                    aurEff->SetAmount(GetAmount() * 4 / 3);
                                    break;
                                default:  // 100% - normal regen
                                    aurEff->SetAmount(GetAmount());
                                    // No need to update after 4th tick
                                    m_isPeriodic = false;
                                    break;
                            }
                            ((Player*)m_target)->UpdateManaRegen();
                        }
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
            case 58549: // Tenacity
            case 59911: // Tenacity (vehicle)
                GetParentAura()->RefreshAura();
                break;
            case 62292: // Blaze (Pool of Tar)
                // should we use custom damage?
                m_target->CastSpell((Unit*)NULL, m_spellProto->EffectTriggerSpell[m_effIndex], true);
                break;
            default:
                break;
        }
        break;
        case SPELLFAMILY_MAGE:
        {
            // Mirror Image
            if (spell->Id == 55342)
            {
                // Set name of summons to name of caster
                m_target->CastSpell((Unit *)NULL, m_spellProto->EffectTriggerSpell[m_effIndex], true);
                m_isPeriodic = false;
            }
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            switch (spell->Id)
            {
                // Demonic Circle
                case 48018:
                    if(GameObject* obj = m_target->GetGameObject(spell->Id))
                    {
                        if (m_target->IsWithinDist(obj, GetSpellMaxRange(48020, true)))
                        {
                            if (!m_target->HasAura(62388))
                                m_target->CastSpell(m_target, 62388, true);
                        }
                        else
                            m_target->RemoveAura(62388);
                    }
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
                // Master of Subtlety
                case 31666:
                    if (!m_target->HasAuraType(SPELL_AURA_MOD_STEALTH))
                        m_target->RemoveAurasDueToSpell(31665);
                    break;
                // Killing Spree
                case 51690:
                {
                    // TODO: this should use effect[1] of 51690
                    std::list<Unit*> targets;
                    {
                        // eff_radius ==0
                        float radius = GetSpellMaxRange(spell, false);

                        CellPair p(Trinity::ComputeCellPair(caster->GetPositionX(),caster->GetPositionY()));
                        Cell cell(p);
                        cell.data.Part.reserved = ALL_DISTRICT;

                        Trinity::AnyUnfriendlyVisibleUnitInObjectRangeCheck u_check(caster, caster, radius);
                        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyVisibleUnitInObjectRangeCheck> checker(caster,targets, u_check);

                        TypeContainerVisitor<Trinity::UnitListSearcher<Trinity::AnyUnfriendlyVisibleUnitInObjectRangeCheck>, GridTypeMapContainer > grid_object_checker(checker);
                        TypeContainerVisitor<Trinity::UnitListSearcher<Trinity::AnyUnfriendlyVisibleUnitInObjectRangeCheck>, WorldTypeMapContainer > world_object_checker(checker);

                        CellLock<GridReadGuard> cell_lock(cell, p);

                        cell_lock->Visit(cell_lock, grid_object_checker,  *caster->GetMap(), *caster, radius);
                        cell_lock->Visit(cell_lock, world_object_checker, *caster->GetMap(), *caster, radius);
                    }

                    if(targets.empty())
                        return;

                    std::list<Unit*>::const_iterator itr = targets.begin();
                    std::advance(itr, rand()%targets.size());
                    Unit* target = *itr;

                    caster->CastSpell(target, 57840, true);
                    caster->CastSpell(target, 57841, true);
                    return;
                }
                // Overkill
                case 58428:
                    if (!m_target->HasAuraType(SPELL_AURA_MOD_STEALTH))
                        m_target->RemoveAurasDueToSpell(58427);
                    break;
//                default:
//                    break;
            }
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Explosive Shot
            if (spell->SpellFamilyFlags[1] & 0x80000000)
            {
                if(caster)
                    caster->CastCustomSpell(53352, SPELLVALUE_BASE_POINT0, m_amount, m_target, true, NULL, this);
                return;
            }
            switch (spell->Id)
            {
                // Feeding Frenzy Rank 1
                case 53511:
                    if ( m_target->getVictim() && m_target->getVictim()->GetHealth() * 100 < m_target->getVictim()->GetMaxHealth() * 35 )
                        m_target->CastSpell(m_target, 60096, true, 0, this);
                    return;
                // Feeding Frenzy Rank 2
                case 53512:
                    if ( m_target->getVictim() && m_target->getVictim()->GetHealth() * 100 < m_target->getVictim()->GetMaxHealth() * 35 )
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
            switch (spell->Id)
            {
                case 49016: //Hysteria
                    uint32 damage = uint32(m_target->GetMaxHealth()*0.01f);
                    m_target->DealDamage(m_target, damage, NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    break;
            }

            // Death and Decay
            if (spell->SpellFamilyFlags[0] & 0x20)
            {
                if (caster)
                    caster->CastCustomSpell(m_target, 52212, &m_amount, NULL, NULL, true, 0, this);
                return;
            }
            // Chains of Ice
            if (spell->SpellFamilyFlags[1] & 0x00004000)
            {
                // Get 0 effect aura
                if (AuraEffect *slow = GetParentAura()->GetPartAura(0))
                {
                    slow->ApplyModifier(false, true);
                    slow->SetAmount(slow->GetAmount() + GetAmount());
                    if (slow->GetAmount() > 0) slow->SetAmount(0);
                    slow->ApplyModifier(true, true);
                }
                return;
            }
            // Summon Gargoyle
            // Being pursuaded by Gargoyle - AI related?
            // if (spell->SpellFamilyFlags[1] & 0x00000080)
            // break;
            // Blood of the North
            // Reaping
            // Death Rune Mastery
            if (spell->SpellIconID == 3041 || spell->SpellIconID == 22 || spell->SpellIconID == 2622)
            {
                if (m_target->GetTypeId() != TYPEID_PLAYER)
                    return;
                // Aura not used - prevent removing death runes from other effects
                if (!GetAmount())
                    return;
                if(((Player*)m_target)->getClass() != CLASS_DEATH_KNIGHT)
                    return;

                // Remove death rune added on proc
                for (uint8 i = 0; i < MAX_RUNES && m_amount; ++i)
                {
                    if (m_spellProto->SpellIconID == 2622)
                    {
                        if (((Player*)m_target)->GetCurrentRune(i) != RUNE_DEATH ||
                            ((Player*)m_target)->GetBaseRune(i) == RUNE_BLOOD)
                            continue;
                    }
                    else
                    {
                        if (((Player*)m_target)->GetCurrentRune(i) != RUNE_DEATH ||
                            ((Player*)m_target)->GetBaseRune(i) != RUNE_BLOOD)
                            continue;
                    }

                    if (!(m_amount & (1<<i)))
                        continue;

                    ((Player*)m_target)->ConvertRune(i,((Player*)m_target)->GetBaseRune(i));
                }
                m_amount = 0;
                return;
            }
            break;
        }
        default:
            break;
    }
}

void AuraEffect::HandlePreventFleeing(bool apply, bool Real, bool /*changeAmount*/)
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

void AuraEffect::HandleArenaPreparation(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;

    if(apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION);
}

/**
 * Such auras are applied from a caster(=player) to a vehicle.
 * This has been verified using spell #49256
 */
void AuraEffect::HandleAuraControlVehicle(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;

    if(!m_target->IsVehicle())
        return;

    Unit *caster = GetParentAura()->GetUnitSource();
    if(!caster || caster == m_target)
        return;

    if (apply)
    {
        //if(caster->GetTypeId() == TYPEID_PLAYER)
        //    if(Pet *pet = ((Player*)caster)->GetPet())
        //        pet->Remove(PET_SAVE_AS_CURRENT);
        caster->EnterVehicle(m_target->GetVehicleKit(), m_amount - 1);
    }
    else
    {
        if(GetId() == 53111) // Devour Humanoid
        {
            m_target->Kill(caster);
            if(caster->GetTypeId() == TYPEID_UNIT)
                ((Creature*)caster)->RemoveCorpse();
        }

        // some SPELL_AURA_CONTROL_VEHICLE auras have a dummy effect on the player - remove them
        caster->RemoveAurasDueToSpell(GetId());
        caster->ExitVehicle();
    }
}

void AuraEffect::HandleAuraConvertRune(bool apply, bool Real, bool changeAmount)
{
    if(!Real && !changeAmount)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *plr = (Player*)m_target;

    if(plr->getClass() != CLASS_DEATH_KNIGHT)
        return;

    uint32 runes = 0;
    // convert number of runes specified in aura amount of rune type in miscvalue to runetype in miscvalueb
    for (uint32 i = 0; i < MAX_RUNES && m_amount; ++i)
    {
        if(apply)
        {
            if (GetMiscValue() != plr->GetCurrentRune(i))
                continue;
            if(!plr->GetRuneCooldown(i))
            {
                plr->ConvertRune(i, RuneType(GetSpellProto()->EffectMiscValueB[m_effIndex]));
                runes |= 1<<i;
                --m_amount;
            }
        }
        else
        {
            if(plr->GetCurrentRune(i) == RuneType(GetSpellProto()->EffectMiscValueB[m_effIndex]))
            {
                if (m_amount & (1<<i))
                    plr->ConvertRune(i, plr->GetBaseRune(i));
            }
        }
    }
    if (apply)
        m_amount = runes;
}

// Control Auras

void AuraEffect::HandleAuraModStun(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;

    m_target->SetControlled(apply, UNIT_STAT_STUNNED);
}

void AuraEffect::HandleAuraModRoot(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;

    m_target->SetControlled(apply, UNIT_STAT_ROOT);
}

// Charm Auras

void AuraEffect::HandleModPossess(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;

    Unit* caster = GetCaster();
    if(caster && caster->GetTypeId() == TYPEID_UNIT)
    {
        HandleModCharm(apply, Real, false);
        return;
    }

    if(apply)
        m_target->SetCharmedBy(caster, CHARM_TYPE_POSSESS);
    else
        m_target->RemoveCharmedBy(caster);
}

// only one spell has this aura
void AuraEffect::HandleModPossessPet(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;

    Unit* caster = GetCaster();
    if(!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    //seems it may happen that when removing it is no longer owner's pet
    //if(((Player*)caster)->GetPet() != m_target)
    //    return;

    if(apply)
    {
        if(((Player*)caster)->GetPet() != m_target)
            return;

        m_target->SetCharmedBy(caster, CHARM_TYPE_POSSESS);
    }
    else
    {
        m_target->RemoveCharmedBy(caster);

        // Reinitialize the pet bar and make the pet come back to the owner
        ((Player*)caster)->PetSpellInitialize();
        if(!m_target->getVictim())
        {
            m_target->GetMotionMaster()->MoveFollow(caster, PET_FOLLOW_DIST, m_target->GetFollowAngle());
            //if(m_target->GetCharmInfo())
            //    m_target->GetCharmInfo()->SetCommandState(COMMAND_FOLLOW);
        }
    }
}

void AuraEffect::HandleModCharm(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;

    Unit* caster = GetCaster();

    if(apply)
        m_target->SetCharmedBy(caster, CHARM_TYPE_CHARM);
    else
        m_target->RemoveCharmedBy(caster);
}

void AuraEffect::HandleCharmConvert(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;

    Unit* caster = GetCaster();

    if(apply)
        m_target->SetCharmedBy(caster, CHARM_TYPE_CONVERT);
    else
        m_target->RemoveCharmedBy(caster);
}

void AuraEffect::HandlePhase(bool apply, bool Real, bool /*changeAmount*/)
{
    if(!Real)
        return;

    // no-phase is also phase state so same code for apply and remove

    // phase auras normally not expected at BG but anyway better check
    if(m_target->GetTypeId() == TYPEID_PLAYER)
    {
        // drop flag at invisible in bg
        if(((Player*)m_target)->InBattleGround())
            if(BattleGround *bg = ((Player*)m_target)->GetBattleGround())
                bg->EventPlayerDroppedFlag((Player*)m_target);

        // GM-mode have mask 0xFFFFFFFF
        if(!((Player*)m_target)->isGameMaster())
            m_target->SetPhaseMask(apply ? GetMiscValue() : PHASEMASK_NORMAL,false);

        ((Player*)m_target)->GetSession()->SendSetPhaseShift(apply ? GetMiscValue() : PHASEMASK_NORMAL);
    }
    else
        m_target->SetPhaseMask(apply ? GetMiscValue() : PHASEMASK_NORMAL,false);

    // need triggering visibility update base at phase update of not GM invisible (other GMs anyway see in any phases)
    if(m_target->GetVisibility()!=VISIBILITY_OFF)
        m_target->SetVisibility(m_target->GetVisibility());
}

void AuraEffect::HandleAuraInitializeImages( bool Apply, bool Real , bool /*changeAmount*/)
{
    if (!Real)
        return;
    if (Apply)
    {
        Unit * caster = GetCaster();
        if (!caster)
            return;
        // Set item visual
        if (caster->GetTypeId()== TYPEID_PLAYER)
        {
            if (Item const * item = ((Player *)caster)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
                m_target->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, item->GetProto()->ItemId);
            if (Item const * item = ((Player *)caster)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
                m_target->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, item->GetProto()->ItemId);
        }
        else // TYPEID_UNIT
        {
            m_target->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, caster->GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID));
            m_target->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, caster->GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1));
            m_target->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 2, caster->GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 2));
        }
    }
    else
    {
        // Remove equipment visual
        if (m_target->GetTypeId() == TYPEID_PLAYER)
        {
            for (uint8 i = 0; i < 3; ++i)
                m_target->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + i, 0);
        }
        else // TYPEID_UNIT
        {
            ((Creature*)m_target)->LoadEquipment(((Creature*)m_target)->GetEquipmentId());
        }
    }
}

void AuraEffect::HandleAuraCloneCaster( bool Apply, bool Real , bool /*changeAmount*/)
{
    if (!Real)
        return;

    if (Apply)
    {
        Unit * caster = GetCaster();
        if (!caster)
            return;
        // Set display id (probably for portrait?)
        m_target->SetDisplayId(caster->GetDisplayId());
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
    }
    else
    {
        m_target->SetDisplayId(m_target->GetNativeDisplayId());
        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
    }
}

void AuraEffect::HandleAuraModCritPct(bool apply, bool Real, bool changeAmount)
{
    /*
    if(m_target->GetTypeId() != TYPEID_PLAYER)
    {
        m_target->m_baseSpellCritChance += apply ? m_amount:-m_amount;
        return;
    }

    if(Real || changeAmount)
        ((Player*)m_target)->UpdateAllSpellCritChances();

    ((Player*)m_target)->HandleBaseModValue(CRIT_PERCENTAGE,         FLAT_MOD, float (m_amount), apply);
    ((Player*)m_target)->HandleBaseModValue(OFFHAND_CRIT_PERCENTAGE, FLAT_MOD, float (m_amount), apply);
    ((Player*)m_target)->HandleBaseModValue(RANGED_CRIT_PERCENTAGE,  FLAT_MOD, float (m_amount), apply);
    */
    // spells required only Real aura add/remove
    if(!Real)
        return;

    if(m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)m_target)->HandleBaseModValue(CRIT_PERCENTAGE,         FLAT_MOD, float (m_amount), apply);
    ((Player*)m_target)->HandleBaseModValue(OFFHAND_CRIT_PERCENTAGE, FLAT_MOD, float (m_amount), apply);
    ((Player*)m_target)->HandleBaseModValue(RANGED_CRIT_PERCENTAGE,  FLAT_MOD, float (m_amount), apply);

    // included in Player::UpdateSpellCritChance calculation
    ((Player*)m_target)->UpdateAllSpellCritChances();
}

void AuraEffect::HandleAuraLinked(bool apply, bool Real, bool /*changeAmount*/)
{
    if (!Real)
        return;

    if (apply)
    {
        Unit * caster = GetCaster();
        if (!caster)
            return;
        // If amount avalible cast with basepoints (Crypt Fever for example)
        if (m_amount)
            caster->CastCustomSpell(m_target, m_spellProto->EffectTriggerSpell[m_effIndex], &m_amount, NULL, NULL, true, NULL, this);
        else
            caster->CastSpell(m_target, m_spellProto->EffectTriggerSpell[m_effIndex],true, NULL, this);
    }
    else
        m_target->RemoveAura(m_spellProto->EffectTriggerSpell[m_effIndex], GetCasterGUID(), AuraRemoveMode(GetParentAura()->GetRemoveMode()));
}

int32 AuraEffect::CalculateCrowdControlAuraAmount(Unit * caster)
{
    // Damage cap for CC effects
    if (!m_spellProto->procFlags)
        return 0;

    if (m_auraName !=SPELL_AURA_MOD_CONFUSE &&
        m_auraName !=SPELL_AURA_MOD_FEAR &&
        m_auraName !=SPELL_AURA_MOD_STUN &&
        m_auraName !=SPELL_AURA_MOD_ROOT &&
        m_auraName !=SPELL_AURA_TRANSFORM)
        return 0;

    int32 damageCap = (int32)(m_target->GetMaxHealth()*0.10f);

    if (!caster)
        return damageCap;

    // Glyphs increasing damage cap
    Unit::AuraEffectList const& overrideClassScripts = caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (Unit::AuraEffectList::const_iterator itr = overrideClassScripts.begin(); itr != overrideClassScripts.end(); ++itr)
    {
        if((*itr)->isAffectedOnSpell(m_spellProto))
        {
            // Glyph of Fear, Glyph of Frost nova and similar auras
            if ((*itr)->GetMiscValue() == 7801)
            {
                damageCap += (int32)(damageCap*(*itr)->GetAmount()/100.0f);
                break;
            }
        }
    }
    return damageCap;
}

bool AuraEffect::IsPeriodicTickCrit(Unit const * pCaster) const
{
    Unit::AuraEffectList const& mPeriodicCritAuras= pCaster->GetAurasByType(SPELL_AURA_ABILITY_PERIODIC_CRIT);
    for (Unit::AuraEffectList::const_iterator itr = mPeriodicCritAuras.begin(); itr != mPeriodicCritAuras.end(); ++itr)
    {
        if ((*itr)->isAffectedOnSpell(m_spellProto) && pCaster->isSpellCrit(m_target, m_spellProto, GetSpellSchoolMask(m_spellProto)))
            return true;
    }
    return false;
}
