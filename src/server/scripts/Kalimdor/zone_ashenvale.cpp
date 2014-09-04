/*
 *
 * Copyright (C) 2011-2014 ArkCORE <http://www.arkania.net/>
 *
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 *
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Ashenvale
SD%Complete: 70
SDComment: Quest support: 6544, 6482
SDCategory: Ashenvale Forest
EndScriptData */

/* ContentData
npc_torek
npc_ruul_snowhoof
EndContentData */

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "GridNotifiers.h"
#include "ScriptedEscortAI.h"
#include "Player.h"
#include "Vehicle.h"

enum ashenvale
{
	QUEST_OF_THEIR_OWN_DESIGN						= 13595,
	NPC_BATHRANS_CORPSE								= 33183,

	QUEST_BATHED_IN_LIGHT							= 13642,
	GO_LIGHT_OF_ELUNE_AT_LAKE_FALATHIM				= 194651,
	ITEM_UNBATHED_CONCOCTION						= 45065,
	ITEM_BATHED_CONCOCTION							= 45066,

	QUEST_RESPECT_FOR_THE_FALLEN					= 13626,
	SPELL_CREATE_FEEROS_HOLY_HAMMER_COVER			= 62837,
	ITEM_FEEROS_HOLY_HAMMER							= 45042,
	SPELL_CREATE_THE_PURIFIERS_PRAYER_BOOK_COVER	= 62839,
	ITEM_PURIFIERS_PRAYER_BOOK						= 45043,

};

/*####
# npc_torek
####*/

enum Torek
{
    SAY_READY                  = 0,
    SAY_MOVE                   = 1,
    SAY_PREPARE                = 2,
    SAY_WIN                    = 3,
    SAY_END                    = 4,
    SPELL_REND                 = 11977,
    SPELL_THUNDERCLAP          = 8078,
    QUEST_TOREK_ASSULT         = 6544,
    NPC_SPLINTERTREE_RAIDER    = 12859,
    NPC_DURIEL                 = 12860,
    NPC_SILVERWING_SENTINEL    = 12896,
    NPC_SILVERWING_WARRIOR     = 12897,
    FACTION_QUEST              = 113
};

class npc_torek : public CreatureScript
{
public:
    npc_torek() : CreatureScript("npc_torek") { }

    struct npc_torekAI : public npc_escortAI
    {
        npc_torekAI(Creature* creature) : npc_escortAI(creature) { }

        void Reset() override
        {
            rend_Timer        = 5000;
            thunderclap_Timer = 8000;
            _completed        = false;
        }

        void EnterCombat(Unit* /*who*/) override { }

        void JustSummoned(Creature* summoned) override
        {
            summoned->AI()->AttackStart(me);
        }

        void sQuestAccept(Player* player, Quest const* quest)
        {
            if (quest->GetQuestId() == QUEST_TOREK_ASSULT)
            {
                /// @todo find companions, make them follow Torek, at any time (possibly done by core/database in future?)
                Talk(SAY_READY, player);
                me->setFaction(FACTION_QUEST);
                npc_escortAI::Start(true, true, player->GetGUID());
            }
        }

        void WaypointReached(uint32 waypointId) override
        {
            if (Player* player = GetPlayerForEscort())
            {
                switch (waypointId)
                {
                    case 1:
                        Talk(SAY_MOVE, player);
                        break;
                    case 8:
                        Talk(SAY_PREPARE, player);
                        break;
                    case 19:
                        /// @todo verify location and creatures amount.
                        me->SummonCreature(NPC_DURIEL, 1776.73f, -2049.06f, 109.83f, 1.54f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                        me->SummonCreature(NPC_SILVERWING_SENTINEL, 1774.64f, -2049.41f, 109.83f, 1.40f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                        me->SummonCreature(NPC_SILVERWING_WARRIOR, 1778.73f, -2049.50f, 109.83f, 1.67f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                        break;
                    case 20:
                        Talk(SAY_WIN, player);
                        _completed = true;
                        player->GroupEventHappens(QUEST_TOREK_ASSULT, me);
                        break;
                    case 21:
                        Talk(SAY_END, player);
                        break;
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);

            if (!UpdateVictim())
                return;

            if (rend_Timer <= diff)
            {
                DoCastVictim(SPELL_REND);
                rend_Timer = 20000;
            } else rend_Timer -= diff;

            if (thunderclap_Timer <= diff)
            {
                DoCast(me, SPELL_THUNDERCLAP);
                thunderclap_Timer = 30000;
            } else thunderclap_Timer -= diff;
        }

    private:
        uint32 rend_Timer;
        uint32 thunderclap_Timer;
        bool   _completed;

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_torekAI(creature);
    }
};

/*####
# npc_ruul_snowhoof
####*/

enum RuulSnowhoof
{
    NPC_THISTLEFUR_URSA         = 3921,
    NPC_THISTLEFUR_TOTEMIC      = 3922,
    NPC_THISTLEFUR_PATHFINDER   = 3926,
    QUEST_FREEDOM_TO_RUUL       = 6482,
    GO_CAGE                     = 178147
};

Position const RuulSnowhoofSummonsCoord[6] =
{
    { 3449.218018f, -587.825073f, 174.978867f, 4.714445f },
    { 3446.384521f, -587.830872f, 175.186279f, 4.714445f },
    { 3444.218994f, -587.835327f, 175.380600f, 4.714445f },
    { 3508.344482f, -492.024261f, 186.929031f, 4.145029f },
    { 3506.265625f, -490.531006f, 186.740128f, 4.239277f },
    { 3503.682373f, -489.393799f, 186.629684f, 4.349232f }
};

class npc_ruul_snowhoof : public CreatureScript
{
public:
    npc_ruul_snowhoof() : CreatureScript("npc_ruul_snowhoof") { }

    struct npc_ruul_snowhoofAI : public npc_escortAI
    {
        npc_ruul_snowhoofAI(Creature* creature) : npc_escortAI(creature) { }

        void Reset() override
        {
            if (GameObject* Cage = me->FindNearestGameObject(GO_CAGE, 20))
                Cage->SetGoState(GO_STATE_READY);
        }

        void EnterCombat(Unit* /*who*/) override { }

        void JustSummoned(Creature* summoned) override
        {
            summoned->AI()->AttackStart(me);
        }

        void sQuestAccept(Player* player, Quest const* quest)
        {
            if (quest->GetQuestId() == QUEST_FREEDOM_TO_RUUL)
            {
                me->setFaction(FACTION_QUEST);
                npc_escortAI::Start(true, false, player->GetGUID());
            }
        }

        void WaypointReached(uint32 waypointId) override
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 0:
                    me->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
                    if (GameObject* Cage = me->FindNearestGameObject(GO_CAGE, 20))
                        Cage->SetGoState(GO_STATE_ACTIVE);
                    break;
                case 13:
                    me->SummonCreature(NPC_THISTLEFUR_TOTEMIC, RuulSnowhoofSummonsCoord[0], TEMPSUMMON_DEAD_DESPAWN, 60000);
                    me->SummonCreature(NPC_THISTLEFUR_URSA, RuulSnowhoofSummonsCoord[1], TEMPSUMMON_DEAD_DESPAWN, 60000);
                    me->SummonCreature(NPC_THISTLEFUR_PATHFINDER, RuulSnowhoofSummonsCoord[2], TEMPSUMMON_DEAD_DESPAWN, 60000);
                    break;
                case 19:
                    me->SummonCreature(NPC_THISTLEFUR_TOTEMIC, RuulSnowhoofSummonsCoord[3], TEMPSUMMON_DEAD_DESPAWN, 60000);
                    me->SummonCreature(NPC_THISTLEFUR_URSA, RuulSnowhoofSummonsCoord[4], TEMPSUMMON_DEAD_DESPAWN, 60000);
                    me->SummonCreature(NPC_THISTLEFUR_PATHFINDER, RuulSnowhoofSummonsCoord[5], TEMPSUMMON_DEAD_DESPAWN, 60000);
                    break;
                case 21:
                    player->GroupEventHappens(QUEST_FREEDOM_TO_RUUL, me);
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ruul_snowhoofAI(creature);
    }
};

enum Muglash
{
    SAY_MUG_START1          = 0,
    SAY_MUG_START2          = 1,
    SAY_MUG_BRAZIER         = 2,
    SAY_MUG_BRAZIER_WAIT    = 3,
    SAY_MUG_ON_GUARD        = 4,
    SAY_MUG_REST            = 5,
    SAY_MUG_DONE            = 6,
    SAY_MUG_GRATITUDE       = 7,
    SAY_MUG_PATROL          = 8,
    SAY_MUG_RETURN          = 9,

    QUEST_VORSHA            = 6641,

    GO_NAGA_BRAZIER         = 178247,

    NPC_WRATH_RIDER         = 3713,
    NPC_WRATH_SORCERESS     = 3717,
    NPC_WRATH_RAZORTAIL     = 3712,

    NPC_WRATH_PRIESTESS     = 3944,
    NPC_WRATH_MYRMIDON      = 3711,
    NPC_WRATH_SEAWITCH      = 3715,

    NPC_VORSHA              = 12940,
    NPC_MUGLASH             = 12717
};

Position const FirstNagaCoord[3] =
{
    { 3603.504150f, 1122.631104f,  1.635f, 0.0f },        // rider
    { 3589.293945f, 1148.664063f,  5.565f, 0.0f },        // sorceress
    { 3609.925537f, 1168.759521f, -1.168f, 0.0f }         // razortail
};

Position const SecondNagaCoord[3] =
{
    { 3609.925537f, 1168.759521f, -1.168f, 0.0f },        // witch
    { 3645.652100f, 1139.425415f, 1.322f,  0.0f },        // priest
    { 3583.602051f, 1128.405762f, 2.347f,  0.0f }         // myrmidon
};

Position const VorshaCoord = {3633.056885f, 1172.924072f, -5.388f, 0.0f};

class npc_muglash : public CreatureScript
{
public:
    npc_muglash() : CreatureScript("npc_muglash") { }

    struct npc_muglashAI : public npc_escortAI
    {
        npc_muglashAI(Creature* creature) : npc_escortAI(creature) { }

        void Reset() override
        {
            eventTimer = 10000;
            waveId = 0;
            _isBrazierExtinguished = false;
        }

        void EnterCombat(Unit* /*who*/) override
        {
            if (Player* player = GetPlayerForEscort())
                if (HasEscortState(STATE_ESCORT_PAUSED))
                {
                    if (urand(0, 1))
                        Talk(SAY_MUG_ON_GUARD, player);
                    return;
                }
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (HasEscortState(STATE_ESCORT_ESCORTING))
                if (Player* player = GetPlayerForEscort())
                    player->FailQuest(QUEST_VORSHA);
        }

        void JustSummoned(Creature* summoned) override
        {
            summoned->AI()->AttackStart(me);
        }

        void sQuestAccept(Player* player, Quest const* quest)
        {
            if (quest->GetQuestId() == QUEST_VORSHA)
            {
                Talk(SAY_MUG_START1);
                me->setFaction(FACTION_QUEST);
                npc_escortAI::Start(true, false, player->GetGUID());
            }
        }

            void WaypointReached(uint32 waypointId) override
            {
                if (Player* player = GetPlayerForEscort())
                {
                    switch (waypointId)
                    {
                        case 0:
                            Talk(SAY_MUG_START2, player);
                            break;
                        case 24:
                            Talk(SAY_MUG_BRAZIER, player);

                            if (GameObject* go = GetClosestGameObjectWithEntry(me, GO_NAGA_BRAZIER, INTERACTION_DISTANCE*2))
                            {
                                go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                                SetEscortPaused(true);
                            }
                            break;
                        case 25:
                            Talk(SAY_MUG_GRATITUDE);
                            player->GroupEventHappens(QUEST_VORSHA, me);
                            break;
                        case 26:
                            Talk(SAY_MUG_PATROL);
                            break;
                        case 27:
                            Talk(SAY_MUG_RETURN);
                            break;
                    }
                }
            }

            void DoWaveSummon()
            {
                switch (waveId)
                {
                    case 1:
                        me->SummonCreature(NPC_WRATH_RIDER,     FirstNagaCoord[0], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                        me->SummonCreature(NPC_WRATH_SORCERESS, FirstNagaCoord[1], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                        me->SummonCreature(NPC_WRATH_RAZORTAIL, FirstNagaCoord[2], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                        break;
                    case 2:
                        me->SummonCreature(NPC_WRATH_PRIESTESS, SecondNagaCoord[0], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                        me->SummonCreature(NPC_WRATH_MYRMIDON,  SecondNagaCoord[1], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                        me->SummonCreature(NPC_WRATH_SEAWITCH,  SecondNagaCoord[2], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                        break;
                    case 3:
                        me->SummonCreature(NPC_VORSHA, VorshaCoord, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                        break;
                    case 4:
                        SetEscortPaused(false);
                        Talk(SAY_MUG_DONE);
                        break;
                }
            }

            void UpdateAI(uint32 diff) override
            {
                npc_escortAI::UpdateAI(diff);

                if (!me->GetVictim())
                {
                    if (HasEscortState(STATE_ESCORT_PAUSED) && _isBrazierExtinguished)
                    {
                        if (eventTimer < diff)
                        {
                            ++waveId;
                            DoWaveSummon();
                            eventTimer = 10000;
                        }
                        else
                            eventTimer -= diff;
                    }
                    return;
                }
                DoMeleeAttackIfReady();
            }

    private:
        uint32 eventTimer;
        uint8  waveId;
    public:
        bool   _isBrazierExtinguished;

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_muglashAI(creature);
    }
};

class go_naga_brazier : public GameObjectScript
{
    public:
        go_naga_brazier() : GameObjectScript("go_naga_brazier") { }

        bool OnGossipHello(Player* /*player*/, GameObject* go) override
        {
            if (Creature* creature = GetClosestCreatureWithEntry(go, NPC_MUGLASH, INTERACTION_DISTANCE*2))
            {
                if (npc_muglash::npc_muglashAI* pEscortAI = CAST_AI(npc_muglash::npc_muglashAI, creature->AI()))
                {
                    creature->AI()->Talk(SAY_MUG_BRAZIER_WAIT);

                    pEscortAI->_isBrazierExtinguished = true;
                    return false;
                }
            }

            return true;
        }
};

/*####
# spell_potion_of_wildfire
####*/

class spell_potion_of_wildfire : public SpellScriptLoader
{
    public:
        spell_potion_of_wildfire() : SpellScriptLoader("spell_potion_of_wildfire") { }

        class spell_potion_of_wildfire_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_potion_of_wildfire_SpellScript);
			
			void HandleOnHit()
            {
				Player* player = GetCaster()->ToPlayer();
				if (!player) return;
				
				if (player->GetQuestStatus(QUEST_OF_THEIR_OWN_DESIGN) == QUEST_STATUS_INCOMPLETE)				
				{							
					if (Creature* creature = player->FindNearestCreature(NPC_BATHRANS_CORPSE, 10.0f, true))					
					{						
						player->KilledMonsterCredit(NPC_BATHRANS_CORPSE, NULL);	
					}
				}				
            }

			void Register() override
            {                
				OnHit += SpellHitFn(spell_potion_of_wildfire_SpellScript::HandleOnHit);				
            }


        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_potion_of_wildfire_SpellScript();
        }
};

/*####
# spell_unbathed_concoction
####*/

class spell_unbathed_concoction : public SpellScriptLoader
{
    public:
        spell_unbathed_concoction() : SpellScriptLoader("spell_unbathed_concoction") { }

        class spell_unbathed_concoction_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_unbathed_concoction_SpellScript);
			
			
            void Unload() override
            {				
				Player* player = GetCaster()->ToPlayer();
				if (!player) return;
				
				if (player->GetQuestStatus(QUEST_BATHED_IN_LIGHT) == QUEST_STATUS_INCOMPLETE)				
				{							
					if (GameObject* go = player->FindNearestGameObject(GO_LIGHT_OF_ELUNE_AT_LAKE_FALATHIM, 10.0f))					
					{						
						if (player->HasItemCount(ITEM_UNBATHED_CONCOCTION,1))
						{
							player->DestroyItemCount (ITEM_UNBATHED_CONCOCTION,1,true);
							player->AddItem(ITEM_BATHED_CONCOCTION,1);
						}						
					}
				}				
            }

			
			void Register() override {}

        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_unbathed_concoction_SpellScript();
        }
};

/*####
# npc_feero_ironhand
####*/

class npc_feero_ironhand : public CreatureScript
{
    public:
        npc_feero_ironhand() : CreatureScript("npc_feero_ironhand") { }

		bool OnGossipHello(Player* player, Creature* creature) override
		{
			if (!player) return true;

			player->PlayerTalkClass->SendCloseGossip();	

			if (!creature) return true;
					
			if (player->GetQuestStatus(QUEST_RESPECT_FOR_THE_FALLEN) == QUEST_STATUS_INCOMPLETE)				
			{			
				if (!player->HasItemCount(ITEM_FEEROS_HOLY_HAMMER,1))
				{							
					player->CastSpell (creature, SPELL_CREATE_FEEROS_HOLY_HAMMER_COVER);	
					player->AddItem(ITEM_FEEROS_HOLY_HAMMER,1);					
				}						
			}
			return true;
		}
};

/*####
# npc_delgren_the_purifier
####*/

class npc_delgren_the_purifier : public CreatureScript
{
    public:
        npc_delgren_the_purifier() : CreatureScript("npc_delgren_the_purifier") { }

		bool OnGossipHello(Player* player, Creature* creature) override
		{
			if (!player) return true;

			player->PlayerTalkClass->SendCloseGossip();	

			if (!creature) return true;
					
			if (player->GetQuestStatus(QUEST_RESPECT_FOR_THE_FALLEN) == QUEST_STATUS_INCOMPLETE)				
			{			
				if (!player->HasItemCount(ITEM_PURIFIERS_PRAYER_BOOK,1))
				{							
					player->CastSpell (creature, SPELL_CREATE_THE_PURIFIERS_PRAYER_BOOK_COVER);						
					player->AddItem(ITEM_PURIFIERS_PRAYER_BOOK,1);
				}						
			}
			return true;
		}
};

//############################################  Quest 13976 Three Friends of the Forest

enum eQuest13976
{
    QUEST_TREE_FRIENDS_OF_THE_FOREST = 13976,
    QUEST_IN_A_BIND = 13982,
    SPELL_BOLYUNS_CAMP_INVISIBILITY_01 = 65709,
    SPELL_BOLYUNS_CAMP_INVISIBILITY_02 = 65710,
    SPELL_BOLYUN_SEE_INVISIBILITY_1 = 65714,
    SPELL_BOLYUN_SEE_INVISIBILITY_2 = 65715,
    SPELL_IN_A_BIND_SEE_INVISIBILITY_SWITCH_1 = 65716, // (trigger 65717)
    SPELL_IN_A_BIND_SEE_INVISIBILITY_SWITCH_2 = 65717,
};

class npc_bolyun_1 : public CreatureScript
{
public:
    npc_bolyun_1() : CreatureScript("npc_bolyun_1") { }

    bool OnQuestReward(Player* player, Creature* /*creature*/, Quest const* quest, uint32 /*opt*/) 
    { 
        if (quest->GetQuestId() == QUEST_IN_A_BIND)
        {
            player->RemoveAura(SPELL_BOLYUN_SEE_INVISIBILITY_1);
            player->AddAura(SPELL_BOLYUN_SEE_INVISIBILITY_2, player);
        }
        return true; 
    }
 
};

//############################################  Quest 13987 The last stand

enum eQuest13987
{
    QUEST_CLEAR_THE_SHRINE = 13985,
    QUEST_THE_LAST_STAND = 13987,
    NPC_BIG_BAOBOB_1 = 34608,
    NPC_BIF_BAOBOB_2 = 34604,
    NPC_DEMONIC_INVADER = 34609,
};

//npc_demonic_invaders

class npc_demonic_invaders : public CreatureScript
{
public:
    npc_demonic_invaders() : CreatureScript("npc_demonic_invaders") { }

    struct npc_demonic_invadersAI : public ScriptedAI
    {
        npc_demonic_invadersAI(Creature *c) : ScriptedAI(c) {}

        uint32  m_timer;
        uint32	m_phase;
        Player*	m_player;

        void Reset() override
        {
            m_phase = 0;
            m_timer = 0;
            m_player = NULL;
        }

        void JustDied(Unit* killer) override
        {
            if (m_player)
                if (Creature* bob = killer->ToCreature())
                    if (m_player->GetQuestStatus(QUEST_THE_LAST_STAND) == QUEST_STATUS_INCOMPLETE)
                        m_player->KilledMonsterCredit(NPC_DEMONIC_INVADER);
        }

        void StartAnim(Player* player)
        {
            m_player = player;
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const  override
    {
        return new npc_demonic_invadersAI(creature);
    }
};

class npc_big_baobob : public CreatureScript
{
public:
    npc_big_baobob() : CreatureScript("npc_big_baobob") { }

    bool OnQuestReward(Player* player, Creature* /*creature*/, Quest const* quest, uint32 /*opt*/)
    {
        if (quest->GetQuestId() == QUEST_CLEAR_THE_SHRINE)
        {
            player->RemoveAura(SPELL_BOLYUNS_CAMP_INVISIBILITY_02);
        }
        return true;
    }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_THE_LAST_STAND)
        {
            if (npc_big_baobob::npc_big_baobobAI* baobobAI = CAST_AI(npc_big_baobob::npc_big_baobobAI, creature->AI()))
            {
                baobobAI->StartAnim(player);
            }
        }
        return true;
    }

    struct npc_big_baobobAI : public ScriptedAI
    {
        npc_big_baobobAI(Creature *creature) : ScriptedAI(creature) {}

        uint32  m_timer;
        uint32  m_phase;
        uint32  m_cooldown;
        Player* m_player;

		void Reset() override
		{
            m_timer = 0;
            m_phase = 0;
            m_cooldown = 0;
            m_player = NULL;
		}

        void AttackStart(Unit* who) override
        {
            AttackStartNoMove(who);
        }

        void StartAnim(Player* player)
        {
            if (m_phase == 0)
            {
                m_player = player;
                m_timer = 1000;
                m_phase = 1;
            }
        }

		void UpdateAI(uint32 diff) override
        {	
            if (m_timer <= diff)
            {
                m_timer = 1000;
                DoWork();
            }
            else
                m_timer -= diff;

            if (!UpdateVictim())			
				return;								
			 
			DoMeleeAttackIfReady();			
        }

        void DoWork()
        {
            m_cooldown++;
            if (m_cooldown > 600)
                Reset();

            if (m_player)
                if (m_player->GetQuestStatus(QUEST_THE_LAST_STAND) == QUEST_STATUS_INCOMPLETE)
                {
                    Creature* demonic = me->FindNearestCreature(NPC_DEMONIC_INVADER, 40.0f, true);
                    if (!demonic)
                    {
                        Position pos = me->GetRandomNearPosition(40.0f);
                        if (Creature* invader = me->SummonCreature(NPC_DEMONIC_INVADER, pos))
                            if (npc_demonic_invaders::npc_demonic_invadersAI* invaderAI = CAST_AI(npc_demonic_invaders::npc_demonic_invadersAI, invader->AI()))
                            {
                                invaderAI->StartAnim(m_player);
                                invader->Attack(me, true);
                            }

                        if (!m_player->IsInCombat())
                        {
                            pos = me->GetRandomNearPosition(40.0f);
                            if (Creature* invader = me->SummonCreature(NPC_DEMONIC_INVADER, pos))
                                if (npc_demonic_invaders::npc_demonic_invadersAI* invaderAI = CAST_AI(npc_demonic_invaders::npc_demonic_invadersAI, invader->AI()))
                                {
                                    invaderAI->StartAnim(m_player);
                                    invader->Attack(m_player, true);
                                }
                        }
                    }
                    else
                        demonic->Attack(me, true);

                }
                else
                    Reset();
        }
   };

	CreatureAI* GetAI(Creature* creature) const  override
    {
        return new npc_big_baobobAI (creature);
    }
};

//############################################  Quest 13624 A Squad of Your Own

enum eQuest13624
{
    QUEST_A_SQUAD_OF_YOU_OWN = 13624,
    NPC_SENTINEL_ONAEYA = 11806,
    NPC_MAESTRAS_POST_SENTINEL = 33338,
    SELL_SUMMON_SENTINELS = 62830,
    SPELL_DRINK_HEALING_POTION = 62822,
    SPELL_SHOOT = 62818,
};

class npc_maestras_post_sentinel : public CreatureScript
{
public:
    npc_maestras_post_sentinel() : CreatureScript("npc_maestras_post_sentinel") { }

    struct npc_maestras_post_sentinelAI : public npc_escortAI
    {
        npc_maestras_post_sentinelAI(Creature* creature) : npc_escortAI(creature) { }

        uint32  m_timer;
        Player* m_player;

        void Reset() override
        {
            m_timer = 1000;
            if (m_player = me->FindNearestPlayer(10.0f, true))
            {
                npc_escortAI::Start(false, false, m_player->GetGUID());
            }
        }

        void DamageTaken(Unit* attacker, uint32& damage) override
        {
            if (me->GetHealthPct() < 25)
                me->CastSpell(me, SPELL_DRINK_HEALING_POTION);
        }

        void WaypointReached(uint32 waypointId) override {}

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);

            if (m_timer <= diff)
            {
                m_timer = 1000;
                if (m_player)
                    if (m_player->GetQuestStatus(QUEST_A_SQUAD_OF_YOU_OWN) == QUEST_STATUS_REWARDED)
                        me->DespawnOrUnsummon();
            }
            else
                m_timer -= diff;

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new  npc_maestras_post_sentinelAI(creature);
    }
};

// maybe someone can make a bether squad
class npc_sentinel_onaeya : public CreatureScript
{
public:
    npc_sentinel_onaeya() : CreatureScript("npc_sentinel_onaeya") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_A_SQUAD_OF_YOU_OWN)
        {
            for (uint8 i = 0; i < 5; ++i)
            {
                std::list<Creature*> sentinels;
                creature->GetCreatureListWithEntryInGrid(sentinels, NPC_MAESTRAS_POST_SENTINEL, 30.0f);
                if (sentinels.size() < 5)
                    player->CastSpell(player, SELL_SUMMON_SENTINELS);
            }
        }
        return true;
    }
};

//############################################ 2 accept quest give aura "astranaar burning: see invis 01"

enum eNpc3847
{
    QUEST_TO_RAENE_WOLFRUNNER = 13645,
    QUEST_ORENDILS_CURE = 26474,
    NPC_ORLENDIL_BROADLEAF = 3847,
    SPELL_ASTRANAARS_BURNING_SEE_INVISIBILITY_01 = 64572,
};

class npc_orendil_broadleaf : public CreatureScript
{
public:
    npc_orendil_broadleaf() : CreatureScript("npc_orendil_broadleaf") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_TO_RAENE_WOLFRUNNER || quest->GetQuestId() == QUEST_ORENDILS_CURE)
        {
            player->AddAura(SPELL_ASTRANAARS_BURNING_SEE_INVISIBILITY_01, player);
        }
        return true;
    }
};

enum eNpc33452
{
    NPC_AVANAS_NIGHTSABER = 33452,
    SPELL_RIDING_NIGHTSABER_TO_ASTRANAAR = 63021,
};

// we need the correct vehicle id for avanas_nightsaber
class npc_avanas_nightsaber : public CreatureScript
{
public:
    npc_avanas_nightsaber() : CreatureScript("npc_avanas_nightsaber") { }

    struct npc_avanas_nightsaberAI : public ScriptedAI
    {
        npc_avanas_nightsaberAI(Creature *c) : ScriptedAI(c) {}

        void Reset() override
        {
            if (Player* player = me->FindNearestPlayer(10.0f, true))
            {
                me->SetSpeed(MOVE_RUN, 3.0f, true);
                me->SetWalk(false);
                me->CastSpell(player, SPELL_RIDING_NIGHTSABER_TO_ASTRANAAR);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const  override
    {
        return new npc_avanas_nightsaberAI(pCreature);
    }

};

enum eNpc33445
{
    QUEST_ASTRANAAR_BOUND = 13646,
    NPC_SENTINEL_AVANA = 33445,
    // SPELL_ASTRANAARS_BURNING_SEE_INVISIBILITY_01 = 64572,
    SPELL_CHARACTER_FORCE_CAST_FROM_GOSSIP = 63020,
    SPELL_SUMMON_AVANAS_NIGHTSABER = 63022,
    GOSSIP_MENU_OPTION_AVANA = 10339,
    GOSSIP_MENU_NPC_AVANA = 14347,
};

// we need the correct vehicle id for avanas_nightsaber
class npc_sentinel_avana : public CreatureScript
{
public:
    npc_sentinel_avana() : CreatureScript("npc_sentinel_avana") { }

    bool OnGossipHello(Player* player, Creature* creature) 
    { 
        player->PrepareQuestMenu(creature->GetGUID());
        if (player->GetQuestStatus(QUEST_ASTRANAAR_BOUND) > QUEST_STATUS_NONE || player->GetQuestStatus(QUEST_TO_RAENE_WOLFRUNNER) > QUEST_STATUS_NONE)
        {
            player->ADD_GOSSIP_ITEM_DB(GOSSIP_MENU_OPTION_AVANA, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        }
        player->SEND_GOSSIP_MENU(GOSSIP_MENU_NPC_AVANA, creature->GetGUID());
        return true; 
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) 
    { 
        if (action == 1001)
        {
            creature->CastSpell(player, SPELL_CHARACTER_FORCE_CAST_FROM_GOSSIP, true);
        }
        return true; 
    }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_ASTRANAAR_BOUND)
        {
            player->AddAura(SPELL_ASTRANAARS_BURNING_SEE_INVISIBILITY_01, player);
        }
        return true;
    }

    struct npc_sentinel_avanaAI : public ScriptedAI
    {
        npc_sentinel_avanaAI(Creature *c) : ScriptedAI(c) {}

        uint32  m_timer;
        uint32	m_phase;

        void Reset() override
        {
            m_timer = 0;
            m_phase = 0;
        }

        void StartAnim()
        {
            m_timer = 2000;
            m_phase = 1;
        }

        void UpdateAI(uint32 diff) override
        {
            if (m_timer <= diff)
            {
                m_timer = 1000;
                DoWork();
            }
            else
                m_timer -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }

        void DoWork()
        {
            switch (m_phase)
            {
            case 1:
                Talk(0);
                m_phase = 2;
                m_timer = 60000;
                break;
            case 2:
                Reset();
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const  override
    {
        return new npc_sentinel_avanaAI(pCreature);
    }

};

//############################################  Quest 13849 astranaar_burning_fire

enum eQuest13849
{
    QUEST_ASTRANAARS_BURNING = 13849,
    NPC_ASTRANAARS_BURNING_FIRE_BUNNY = 34123,
    SPELL_ASTRANAARS_BURNING_SMOKE = 64565,
    SPELL_THROW_BUCKET_OF_WATER = 64558,
    SPELL_BATHRANS_CORPSE_FIRE = 62511,
};

class npc_astranaar_burning_fire_bunny : public CreatureScript
{
public:
    npc_astranaar_burning_fire_bunny() : CreatureScript("npc_astranaar_burning_fire_bunny") { }

    struct npc_astranaar_burning_fire_bunnyAI : public ScriptedAI
    {
        npc_astranaar_burning_fire_bunnyAI(Creature *c) : ScriptedAI(c) {}		

		uint32  m_timer;
		uint32	m_phase;
		Player*	m_player;

		void Reset() override
		{
			m_phase=0; 
            m_timer=0;
            if (me->HasAura(SPELL_ASTRANAARS_BURNING_SMOKE))
                me->RemoveAura(SPELL_ASTRANAARS_BURNING_SMOKE);
		}

		void SpellHit(Unit* Hitter, SpellInfo const* spell) override  
		{ 
            if (m_player = Hitter->ToPlayer())
            {
                m_phase = 1;
                m_timer = 1000;
            }
		}

		void UpdateAI(uint32 diff) override
        {
            if (m_timer <= diff)
            {
                m_timer = 1000;
                DoWork();
            }
            else
                m_timer -= diff;

            if (!UpdateVictim())			
				return;						
			else 
				DoMeleeAttackIfReady();			
        }

		void DoWork()
		{
			switch (m_phase)
			{
			case 1:
				{
					me->AddAura(SPELL_ASTRANAARS_BURNING_SMOKE, me);
					if (m_player) 
                        m_player->KilledMonsterCredit(NPC_ASTRANAARS_BURNING_FIRE_BUNNY, NULL);	
					m_timer=600;
                    m_phase=2;
					break;
				}
			case 2:
				{
					me->DespawnOrUnsummon();
					m_timer=0;
                    m_phase=0;
					break;
				}
			}
		}
    };

	CreatureAI* GetAI(Creature* pCreature) const  override
    {
        return new npc_astranaar_burning_fire_bunnyAI (pCreature);
    }
};

//############################################  Quest 13853 Return Fire    

enum eQuest13853
{
    QUEST_RETURN_FIRE = 13853,
    NPC_ASTRANAAR_THROWER = 34132,
    NPC_WATCH_WIND_RIDER = 34160,
    NPC_HELLSCREAMS_HELLION = 34163,
    NPC_RETURN_FIRE_KILL_CREDIT_BUNNY = 34176,
    SPELL_ASTRANAAR_THROWER = 68388,
    SPELL_RETURN_TO_ASTRANAAR = 64750,
    SPELL_LAUNCH_GLAIVE = 77744,   // 66289, 66900, 70735, 70736, 70738, 77741, 77742, 77743, 77744, 77749, 84875, 84876, all wrong
};

// we missing the correct spell for glaive shoot
class npc_astranaar_thrower : public CreatureScript
{
public:
    npc_astranaar_thrower() : CreatureScript("npc_astranaar_thrower") { }

    struct npc_astranaar_throwerAI : public VehicleAI
    {
        npc_astranaar_throwerAI(Creature* creature) : VehicleAI(creature) { }

        void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
        {
            // as long the glaive spell is unknown, we set the quest to complete
            if (Player* player = passenger->ToPlayer())
                if (player->GetQuestStatus(QUEST_RETURN_FIRE) == QUEST_STATUS_INCOMPLETE)
                    player->CompleteQuest(QUEST_RETURN_FIRE);
            
            // there are dead sentinels on the seat.. i remove them..
            if (passenger->ToCreature()) 
                passenger->ExitVehicle();
        }

        void UpdateAI(uint32 /*diff*/) override 
        { 
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_astranaar_throwerAI(creature);
    }
};

// we missing the correct spell for glaive shoot
class npc_watch_wind_rider : public CreatureScript
{
public:
    npc_watch_wind_rider() : CreatureScript("npc_watch_wind_rider") { }

    struct npc_watch_wind_riderAI : public VehicleAI
    {
        npc_watch_wind_riderAI(Creature *c) : VehicleAI(c) {}

        void SpellHit(Unit* Hitter, SpellInfo const* spell) override
        {
            
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const  override
    {
        return new npc_watch_wind_riderAI(pCreature);
    }
};

class npc_sentinel_thenysil : public CreatureScript
{
public:
    npc_sentinel_thenysil() : CreatureScript("npc_sentinel_thenysil") { }

    bool OnQuestReward(Player* player, Creature* /*creature*/, Quest const* quest, uint32 /*opt*/) 
    { 
        if (player && quest->GetQuestId() == QUEST_RETURN_FIRE)
            player->RemoveAura(SPELL_ASTRANAARS_BURNING_SEE_INVISIBILITY_01);

        return true; 
    }
};

class npc_benjari_edune : public CreatureScript
{
public:
    npc_benjari_edune() : CreatureScript("npc_benjari_edune") { }

    struct npc_benjari_eduneAI : public ScriptedAI
    {
        npc_benjari_eduneAI(Creature *c) : ScriptedAI(c) {}

        uint32 m_timer;
        uint32 m_phase;

        void Reset() override
        {
            m_timer = 0;
            m_phase = 0;
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (m_phase == 0)
                if (Player* player = who->ToPlayer())
                    if (player->GetDistance2d(me) < 10.0f)
                    {
                        m_phase = urand(1, 5);
                        m_timer = 5000;
                    }
        }

        void UpdateAI(uint32 diff) override
        {
            if (m_timer <= diff)
            {
                m_timer = 5000;
                DoWork();
            }
            else
                m_timer -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }

        void DoWork()
        {
            switch (m_phase)
            {
            case 1:
                Talk(10); m_phase = 6;
                break;
            case 2:
                Talk(1); m_phase = 7;
                break;
            case 3:
                Talk(7); m_phase = 8;
                break;
            case 4:
                Talk(8); m_phase = 9;
                break;
            case 5:
                m_timer = 8000;
                Talk(9); m_phase = 10;
                break;
            case 6:
                Talk(5); m_phase = 11;
                break;
            case 7:
                Talk(2); m_phase = 11;
                break;
            case 8:
                Talk(6); m_phase = 11;
                break;
            case 9:
                Talk(4); m_phase = 11;
                break;
            case 10:
                Talk(3); m_phase = 11;
                break;
            case 11:
                m_timer = 60000;
                m_phase = 12;
                break;
            case 12:
                Reset();
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const  override
    {
        return new npc_benjari_eduneAI(pCreature);
    }

};


void AddSC_ashenvale()
{
    new npc_torek();
    new npc_ruul_snowhoof();
    new npc_muglash();
    new go_naga_brazier();
	new spell_potion_of_wildfire();
	new spell_unbathed_concoction();
	new npc_feero_ironhand();
	new npc_delgren_the_purifier();
	new npc_bolyun_1();
	new npc_big_baobob();
    new npc_demonic_invaders();
    new npc_orendil_broadleaf();
    new npc_sentinel_avana();
	new npc_astranaar_burning_fire_bunny();
    new npc_sentinel_onaeya();
    new npc_maestras_post_sentinel();
    new npc_avanas_nightsaber();
    new npc_astranaar_thrower();
    new npc_watch_wind_rider();
    new npc_sentinel_thenysil();
    new npc_benjari_edune();
}
