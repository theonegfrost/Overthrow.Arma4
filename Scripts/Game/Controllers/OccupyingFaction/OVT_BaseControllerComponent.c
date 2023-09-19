class OVT_BaseControllerComponentClass: OVT_ComponentClass
{
};

class OVT_BaseControllerComponent: OVT_Component
{
	[Attribute(defvalue: "0", UIWidgets.EditBox, desc: "Resources to allocate for testing only")]
	int m_iTestingResources;
	
	[Attribute("", UIWidgets.Object)]
	ref array<ref OVT_BaseUpgrade> m_aBaseUpgrades;
				
	string m_sName;	
	
	ref array<ref EntityID> m_AllSlots;
	ref array<ref EntityID> m_AllCloseSlots;
	ref array<ref EntityID> m_SmallSlots;
	ref array<ref EntityID> m_MediumSlots;
	ref array<ref EntityID> m_LargeSlots;
	ref array<ref EntityID> m_SmallRoadSlots;
	ref array<ref EntityID> m_MediumRoadSlots;
	ref array<ref EntityID> m_LargeRoadSlots;
	ref array<ref EntityID> m_Parking;
	ref array<ref EntityID> m_aSlotsFilled;
	ref array<ref vector> m_aDefendPositions;
		
	protected OVT_OccupyingFactionManager m_occupyingFactionManager;
	
	protected const int UPGRADE_UPDATE_FREQUENCY = 10000;
	
	void InitBase()
	{
		if(!Replication.IsServer()) return;
		if (SCR_Global.IsEditMode()) return;
		
		m_occupyingFactionManager = OVT_Global.GetOccupyingFaction();
		
		InitializeBase();
		
		GetGame().GetCallqueue().CallLater(UpdateUpgrades, UPGRADE_UPDATE_FREQUENCY, true, GetOwner());		
		
		SCR_FactionAffiliationComponent affiliation = EPF_Component<SCR_FactionAffiliationComponent>.Find(GetOwner());
		affiliation.GetOnFactionChanged().Insert(OnFactionChanged);
	}
	
	protected void UpdateUpgrades()
	{
		if(!IsOccupyingFaction()) return;
		
		foreach(OVT_BaseUpgrade upgrade : m_aBaseUpgrades)
		{
			upgrade.OnUpdate(UPGRADE_UPDATE_FREQUENCY);
		}
	}
	
	bool IsOccupyingFaction()
	{
		SCR_FactionAffiliationComponent affiliation = EPF_Component<SCR_FactionAffiliationComponent>.Find(GetOwner());
		string occupyingFaction = OVT_Global.GetConfig().GetOccupyingFactionData().GetFactionKey();
		return affiliation.GetAffiliatedFaction().GetFactionKey() == occupyingFaction;
	}
	
	int GetControllingFaction()
	{
		SCR_FactionAffiliationComponent affiliation = EPF_Component<SCR_FactionAffiliationComponent>.Find(GetOwner());
		
		return GetGame().GetFactionManager().GetFactionIndex(affiliation.GetAffiliatedFaction());
	}
	
	void SetControllingFaction(string key, bool suppressEvents = false)
	{
		FactionManager mgr = GetGame().GetFactionManager();
		Faction faction = mgr.GetFactionByKey(key);
		int index = mgr.GetFactionIndex(faction);
		SetControllingFaction(index, suppressEvents);
	}
	
	void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction newFaction)
	{
		SlotManagerComponent slots = EPF_Component<SlotManagerComponent>.Find(GetOwner());
		EntitySlotInfo slot = slots.GetSlotByName("Flag");
		IEntity flag = slot.GetAttachedEntity();
		SCR_EntityHelper.DeleteEntityAndChildren(flag);
		OVT_Faction fac = OVT_Global.GetFactions().GetOverthrowFactionByKey(newFaction.GetFactionKey());
		IEntity newFlag = GetGame().SpawnEntityPrefab(Resource.Load(fac.m_sFlagPrefab));
		slot.AttachEntity(newFlag);		
	}
	
	void SetControllingFaction(int index, bool suppressEvents = false)
	{		
		if(!suppressEvents)
			m_occupyingFactionManager.OnBaseControlChange(this);
		
		Faction fac = GetGame().GetFactionManager().GetFactionByIndex(index);
		SCR_FactionAffiliationComponent affiliation = EPF_Component<SCR_FactionAffiliationComponent>.Find(GetOwner());
		affiliation.SetAffiliatedFaction(fac);					
	}
	
	void InitializeBase()
	{		
		m_AllSlots = new array<ref EntityID>;
		m_AllCloseSlots = new array<ref EntityID>;
		m_SmallSlots = new array<ref EntityID>;
		m_MediumSlots = new array<ref EntityID>;
		m_LargeSlots = new array<ref EntityID>;
		m_SmallRoadSlots = new array<ref EntityID>;
		m_MediumRoadSlots = new array<ref EntityID>;
		m_LargeRoadSlots = new array<ref EntityID>;
		m_Parking = new array<ref EntityID>;
		m_aSlotsFilled = new array<ref EntityID>;
		m_aDefendPositions = new array<ref vector>;
		
		FindSlots();
		FindParking();
		
		foreach(OVT_BaseUpgrade upgrade : m_aBaseUpgrades)
		{
			upgrade.Init(this, m_occupyingFactionManager, OVT_Global.GetConfig());
		}
		
		//Spend testing resources (if any)
		if(m_iTestingResources > 0){
			SpendResources(m_iTestingResources);
		}
	}
	
	OVT_BaseUpgrade FindUpgrade(string type, string tag = "")
	{
		foreach(OVT_BaseUpgrade upgrade : m_aBaseUpgrades)
		{
			if(tag != "")
			{
				OVT_BaseUpgradeComposition comp = OVT_BaseUpgradeComposition.Cast(upgrade);
				if(!comp) continue;
				if(comp.m_sCompositionTag == tag)
				{
					return upgrade;
				}else{
					continue;
				}
			}
			if(upgrade.ClassName() == type) return upgrade;
		}
		return null;
	}
	
	void FindSlots()
	{
		GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(),  OVT_Global.GetConfig().m_Difficulty.baseRange, CheckSlotAddToArray, FilterSlotEntities);
	}
	
	bool FilterSlotEntities(IEntity entity)
	{
		SCR_EditableEntityComponent editable = EPF_Component<SCR_EditableEntityComponent>.Find(entity);
		if(editable && editable.GetEntityType() == EEditableEntityType.SLOT)
		{
			return true;
		}
		
		SCR_AISmartActionSentinelComponent action = EPF_Component<SCR_AISmartActionSentinelComponent>.Find(entity);
		if(action) {
			SCR_MapDescriptorComponent mapdes = EPF_Component<SCR_MapDescriptorComponent>.Find(entity);
			if(mapdes)
			{
				EMapDescriptorType type = mapdes.GetBaseType();
				if(type == EMapDescriptorType.MDT_TOWER) return false; //Towers are handled by OVT_BaseUpgradeTowerGuard
			}
			return true;
		}
		return false;
	}
	
	bool CheckSlotAddToArray(IEntity entity)
	{
		SCR_AISmartActionSentinelComponent action = EPF_Component<SCR_AISmartActionSentinelComponent>.Find(entity);
		if(action)
		{
			vector pos = entity.GetOrigin();
			if(!m_aDefendPositions.Contains(pos))
				m_aDefendPositions.Insert(entity.GetOrigin());
			return true;
		}
		
		SCR_EditableEntityComponent editable = EPF_Component<SCR_EditableEntityComponent>.Find(entity);
		if(editable && editable.GetEntityType() == EEditableEntityType.SLOT)
		{		
			SCR_EditableEntityUIInfo uiinfo = SCR_EditableEntityUIInfo.Cast(editable.GetInfo());
			if(!uiinfo) return true;
									
			m_AllSlots.Insert(entity.GetID());
			
			float distance = vector.Distance(entity.GetOrigin(), GetOwner().GetOrigin());
			if(distance <  OVT_Global.GetConfig().m_Difficulty.baseCloseRange)
			{
				m_AllCloseSlots.Insert(entity.GetID());
			}
			
			string name = entity.GetPrefabData().GetPrefabName();
			if(uiinfo.HasEntityLabel(EEditableEntityLabel.SLOT_FLAT_SMALL)) m_SmallSlots.Insert(entity.GetID());
			if(uiinfo.HasEntityLabel(EEditableEntityLabel.SLOT_FLAT_MEDIUM)) m_MediumSlots.Insert(entity.GetID());
			if(uiinfo.HasEntityLabel(EEditableEntityLabel.SLOT_FLAT_LARGE)) m_LargeSlots.Insert(entity.GetID());
			if(uiinfo.HasEntityLabel(EEditableEntityLabel.SLOT_ROAD_SMALL)) m_SmallRoadSlots.Insert(entity.GetID());
			if(uiinfo.HasEntityLabel(EEditableEntityLabel.SLOT_ROAD_MEDIUM)) m_MediumRoadSlots.Insert(entity.GetID());
			if(uiinfo.HasEntityLabel(EEditableEntityLabel.SLOT_ROAD_LARGE)) m_LargeRoadSlots.Insert(entity.GetID());
		}
		
		return true;
	}
	
	void FindParking()
	{
		GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), OVT_Global.GetConfig().m_Difficulty.baseCloseRange, null, FilterParkingEntities, EQueryEntitiesFlags.ALL);
	}
	
	bool FilterParkingEntities(IEntity entity)
	{
		if(entity.FindComponent(OVT_ParkingComponent)) {
			m_Parking.Insert(entity.GetID());
		}
		return false;
	}
	
	int SpendResources(int resources, float threat = 0)
	{
		int spent = 0;
		
		for(int priority = 1; priority < 20; priority++)
		{
			if(resources <= 0) break;
			foreach(OVT_BaseUpgrade upgrade : m_aBaseUpgrades)
			{
				if(resources <= 0) break;
				if(upgrade.m_iMinimumThreat > threat) continue;
				if(upgrade.m_iPriority == priority)
				{					
					int allocate = upgrade.m_iResourceAllocation * OVT_Global.GetConfig().m_Difficulty.baseResourceCost;
					int newres = 0;
					if(allocate < 0)
					{
						//Ignore allocation, spend recklessly
						newres = upgrade.Spend(resources, threat);
					}else{
						if(resources < allocate) allocate = resources;
						newres = upgrade.SpendToAllocation(threat);
					}
					
					spent += newres;
					resources -= newres;
				}
			}
		}
		
		return spent;
	}
	
	IEntity GetNearestSlot(vector pos)
	{
		IEntity nearest;
		float nearestDist = -1;
		foreach(EntityID id : m_AllSlots)
		{
			IEntity ent = GetGame().GetWorld().FindEntityByID(id);
			float dist = vector.Distance(pos, ent.GetOrigin());
			if(nearestDist == -1 || dist < nearestDist)
			{
				nearest = ent;
				nearestDist = dist;
			}
		}
		return nearest;
	}
	
	//RPC methods
	
	
	
	void ~OVT_BaseControllerComponent()
	{
		if(m_aBaseUpgrades)
		{
			m_aBaseUpgrades.Clear();
			m_aBaseUpgrades = null;
		}
		if(m_AllSlots)
		{
			m_AllSlots.Clear();
			m_AllSlots = null;
		}
		if(m_AllCloseSlots)
		{
			m_AllCloseSlots.Clear();
			m_AllCloseSlots = null;
		}
		if(m_SmallSlots)
		{
			m_SmallSlots.Clear();
			m_SmallSlots = null;
		}
		if(m_MediumSlots)
		{
			m_MediumSlots.Clear();
			m_MediumSlots = null;
		}
		if(m_LargeSlots)
		{
			m_LargeSlots.Clear();
			m_LargeSlots = null;
		}
		if(m_SmallRoadSlots)
		{
			m_SmallRoadSlots.Clear();
			m_SmallRoadSlots = null;
		}
		if(m_MediumRoadSlots)
		{
			m_MediumRoadSlots.Clear();
			m_MediumRoadSlots = null;
		}
		if(m_LargeRoadSlots)
		{
			m_LargeRoadSlots.Clear();
			m_LargeRoadSlots = null;
		}
		if(m_Parking)
		{
			m_Parking.Clear();
			m_Parking = null;
		}
	}
}