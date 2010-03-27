/* Script Data Start
SDName: Boss svala
SDAuthor: Tartalo
SD%Complete:
SDComment:
SDCategory:
Script Data End */

/*** SQL START ***
update creature_template set scriptname = 'boss_svala' where entry = '';
*** SQL END ***/
#include "ScriptedPch.h"
#include "utgarde_pinnacle.h"

enum Spells
{
    SPELL_CALL_FLAMES                        = 48258,
    SPELL_RITUAL_OF_THE_SWORD                = 48276, //Effect #1 Teleport,  Effect #2 Dummy
    SPELL_SINSTER_STRIKE                     = 15667,
    H_SPELL_SINSTER_STRIKE                   = 59409,
    SPELL_SVALA_TRANSFORMING1                = 54140,
    SPELL_SVALA_TRANSFORMING2                = 54205
};
//not in db
enum Yells
{
    SAY_DIALOG_WITH_ARTHAS_1              = -1575015,
    SAY_DIALOG_WITH_ARTHAS_2              = -1575016,
    SAY_DIALOG_WITH_ARTHAS_3              = -1575017,
    SAY_AGGRO                             = -1575018,
    SAY_SLAY_1                            = -1575019,
    SAY_SLAY_2                            = -1575020,
    SAY_SLAY_3                            = -1575021,
    SAY_DEATH                             = -1575022,
    SAY_SACRIFICE_PLAYER_1                = -1575023,
    SAY_SACRIFICE_PLAYER_2                = -1575024,
    SAY_SACRIFICE_PLAYER_3                = -1575025,
    SAY_SACRIFICE_PLAYER_4                = -1575026,
    SAY_SACRIFICE_PLAYER_5                = -1575027,
    SAY_DIALOG_OF_ARTHAS_1                = -1575028,
    SAY_DIALOG_OF_ARTHAS_2                = -1575029
};
enum Creatures
{
    CREATURE_ARTHAS                          = 24266, // Image of Arthas
    CREATURE_SVALA_SORROWGRAVE               = 26668, // Svala after transformation
    CREATURE_SVALA                           = 29281, // Svala before transformation
    CREATURE_RITUAL_CHANNELER                = 27281
};
enum ChannelerSpells
{
    //ritual channeler's spells
    SPELL_PARALYZE                           = 48278,
    SPELL_SHADOWS_IN_THE_DARK                = 59407
};
enum Misc
{
    DATA_SVALA_DISPLAY_ID                    = 25944
};
enum IntroPhase
{
    IDLE,
    INTRO,
    FINISHED
};
enum CombatPhase
{
    NORMAL,
    SACRIFICING
};
static Position RitualChannelerPos[]=
{
    {296.42, -355.01, 90.94},
    {302.36, -352.01, 90.54},
    {291.39, -350.89, 90.54}
};

struct boss_svalaAI : public ScriptedAI
{
    boss_svalaAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    uint32 uiIntroTimer;

    uint8 uiIntroPhase;

    IntroPhase Phase;

    TempSummon* pArthas;
    uint64 ArthasGUID;

    ScriptedInstance* pInstance;

    void Reset()
    {
        Phase = IDLE;
        uiIntroTimer = 1000;
        uiIntroPhase = 0;
        ArthasGUID = 0;

        if (pInstance)
            pInstance->SetData(DATA_SVALA_SORROWGRAVE_EVENT, NOT_STARTED);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (!pWho)
            return;

        if (Phase == IDLE && pWho->isTargetableForAttack() && m_creature->IsHostileTo(pWho) && m_creature->IsWithinDistInMap(pWho, 40))
        {
            Phase = INTRO;
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            if (Creature *pArthas = m_creature->SummonCreature(CREATURE_ARTHAS, 295.81, -366.16, 92.57, 1.58, TEMPSUMMON_MANUAL_DESPAWN))
            {
                pArthas->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
                pArthas->SetFloatValue(OBJECT_FIELD_SCALE_X, 5);
                ArthasGUID = pArthas->GetGUID();
            }
        }
    }

    void AttackStart(Unit* who) {}

    void UpdateAI(const uint32 diff)
    {
        if (Phase != INTRO)
            return;

        if (uiIntroTimer <= diff)
        {
            Creature *pArthas = Unit::GetCreature(*m_creature, ArthasGUID);
            if (!pArthas)
                return;

            switch (uiIntroPhase)
            {
                case 0:
                    DoScriptText(SAY_DIALOG_WITH_ARTHAS_1, m_creature);
                    ++uiIntroPhase;
                    uiIntroTimer = 3500;
                    break;
                case 1:
                    DoScriptText(SAY_DIALOG_OF_ARTHAS_1, pArthas);
                    ++uiIntroPhase;
                    uiIntroTimer = 3500;
                    break;
                case 2:
                    DoScriptText(SAY_DIALOG_WITH_ARTHAS_2, m_creature);
                    ++uiIntroPhase;
                    uiIntroTimer = 3500;
                    break;
                case 3:
                    DoScriptText(SAY_DIALOG_OF_ARTHAS_2, pArthas);
                    ++uiIntroPhase;
                    uiIntroTimer = 3500;
                    break;
                case 4:
                    DoScriptText(SAY_DIALOG_WITH_ARTHAS_3, m_creature);
                    DoCast(m_creature, SPELL_SVALA_TRANSFORMING1);
                    ++uiIntroPhase;
                    uiIntroTimer = 2800;
                    break;
                case 5:
                    DoCast(m_creature, SPELL_SVALA_TRANSFORMING2);
                    ++uiIntroPhase;
                    uiIntroTimer = 200;
                    break;
                case 6:
                    if (Creature* pSvalaSorrowgrave = m_creature->SummonCreature(CREATURE_SVALA_SORROWGRAVE, 296.632, -346.075, 90.6307, 1.58, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000))
                    {
                        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
                        m_creature->SetDisplayId(DATA_SVALA_DISPLAY_ID);
                        pArthas->ToTempSummon()->UnSummon();
                        ArthasGUID = 0;
                        Phase = FINISHED;
                    }
                    else 
                        Reset();
                    break;
            }
        } else uiIntroTimer -= diff;
    }
};

struct mob_ritual_channelerAI : public Scripted_NoMovementAI
{
    mob_ritual_channelerAI(Creature *c) :Scripted_NoMovementAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    void Reset()
    {
        DoCast(m_creature, SPELL_SHADOWS_IN_THE_DARK);
    }

    // called by svala sorrowgrave to set guid of victim
    void SetGUID(const uint64 &guid, int32 id) 
    {
        if (Unit *pVictim = m_creature->GetUnit(*m_creature, guid))
            DoCast(pVictim, SPELL_PARALYZE);
    }

    void EnterCombat(Unit* who)
    {
    }
};

struct boss_svala_sorrowgraveAI : public ScriptedAI
{
    boss_svala_sorrowgraveAI(Creature *c) : ScriptedAI(c), summons(c)
    {
        pInstance = c->GetInstanceData();
    }

    uint32 uiSinsterStrikeTimer;
    uint32 uiCallFlamesTimer;
    uint32 uiRitualOfSwordTimer;
    uint32 uiSacrificeTimer;

    CombatPhase Phase;

    SummonList summons;
    Unit* pSacrificeTarget;
    
    bool bSacrificed;

    ScriptedInstance* pInstance;

    void Reset()
    {
        uiSinsterStrikeTimer = 7000;
        uiCallFlamesTimer = 10000;
        uiRitualOfSwordTimer = 20000;
        uiSacrificeTimer = 8000;
        
        bSacrificed = false;

        Phase = NORMAL;

        DoTeleportTo(296.632, -346.075, 90.6307);
        m_creature->SetUnitMovementFlags(MOVEMENTFLAG_WALK_MODE);

        summons.DespawnAll();
        pSacrificeTarget = NULL;

        if (pInstance)
            pInstance->SetData(DATA_SVALA_SORROWGRAVE_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit* who)
    {
        DoScriptText(SAY_AGGRO, m_creature);

        if (pInstance)
            pInstance->SetData(DATA_SVALA_SORROWGRAVE_EVENT, IN_PROGRESS);
    }

    void JustSummoned(Creature *summon)
    {
        summons.Summon(summon);
    }

    void SummonedCreatureDespawn(Creature *summon)
    {
        summons.Despawn(summon);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Phase == NORMAL)
        {
            //Return since we have no target
            if (!UpdateVictim())
                return;

            if (uiSinsterStrikeTimer <= diff)
            {
                DoCast(m_creature->getVictim(), DUNGEON_MODE(SPELL_SINSTER_STRIKE, H_SPELL_SINSTER_STRIKE));
                uiSinsterStrikeTimer = urand(5000,9000);
            } else uiSinsterStrikeTimer -= diff;

            if (uiCallFlamesTimer <= diff)
            {
                if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                {
                    DoCast(pTarget, SPELL_CALL_FLAMES);
                    uiCallFlamesTimer = urand(8000,12000);
                }
            } else uiCallFlamesTimer -= diff;

            if (!bSacrificed)
                if (uiRitualOfSwordTimer <= diff)
                {
                    pSacrificeTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true);
                    if (pSacrificeTarget)
                    {
                        DoScriptText(RAND(SAY_SACRIFICE_PLAYER_1,SAY_SACRIFICE_PLAYER_2,SAY_SACRIFICE_PLAYER_3,SAY_SACRIFICE_PLAYER_4,SAY_SACRIFICE_PLAYER_5),m_creature);
                        DoCast(pSacrificeTarget, SPELL_RITUAL_OF_THE_SWORD);
                        //Spell doesn't teleport
                        DoTeleportPlayer(pSacrificeTarget, 296.632, -346.075, 90.63, 4.6);
                        m_creature->SetUnitMovementFlags(MOVEMENTFLAG_FLY_MODE);
                        DoTeleportTo(296.632, -346.075, 120.85);
                        Phase = SACRIFICING;

                        for (uint8 i = 0; i < 3; ++i)
                            if (Creature* pRitualChanneler = m_creature->SummonCreature(CREATURE_RITUAL_CHANNELER, RitualChannelerPos[i], TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 360000))
                                if (pRitualChanneler->AI())
                                    pRitualChanneler->AI()->SetGUID(pSacrificeTarget->GetGUID());

                        bSacrificed = true;
                    }
                } else uiRitualOfSwordTimer -= diff;

            DoMeleeAttackIfReady();
        }
        else  //SACRIFICING
        {
            if (uiSacrificeTimer <= diff)
            {
                if (!summons.empty() && pSacrificeTarget && pSacrificeTarget->isAlive())
                    m_creature->Kill(pSacrificeTarget, false); // durability damage?

                //go down
                Phase = NORMAL;
                pSacrificeTarget = NULL;
                m_creature->SetUnitMovementFlags(MOVEMENTFLAG_WALK_MODE);
                if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    m_creature->GetMotionMaster()->MoveChase(pTarget);

                uiSacrificeTimer = 8000;
            }
            else uiSacrificeTimer -= diff;
        }
    }

    void KilledUnit(Unit* pVictim)
    {
        DoScriptText(RAND(SAY_SLAY_1,SAY_SLAY_2,SAY_SLAY_3), m_creature);
    }

    void JustDied(Unit* pKiller)
    {
        if (pInstance)
        {
            Creature* pSvala = Unit::GetCreature((*m_creature), pInstance->GetData64(DATA_SVALA));
            if (pSvala && pSvala->isAlive())
                pKiller->Kill(pSvala);

            pInstance->SetData(DATA_SVALA_SORROWGRAVE_EVENT, DONE);
        }
        DoScriptText(SAY_DEATH, m_creature);
    }
};

CreatureAI* GetAI_boss_svala(Creature* pCreature)
{
    return new boss_svalaAI (pCreature);
}

CreatureAI* GetAI_mob_ritual_channeler(Creature* pCreature)
{
    return new mob_ritual_channelerAI(pCreature);
}

CreatureAI* GetAI_boss_svala_sorrowgrave(Creature* pCreature)
{
    return new boss_svala_sorrowgraveAI(pCreature);
}

void AddSC_boss_svala()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_svala";
    newscript->GetAI = &GetAI_boss_svala;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ritual_channeler";
    newscript->GetAI = &GetAI_mob_ritual_channeler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_svala_sorrowgrave";
    newscript->GetAI = &GetAI_boss_svala_sorrowgrave;
    newscript->RegisterSelf();
}
