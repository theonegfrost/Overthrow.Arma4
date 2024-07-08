
[BaseContainerProps()]
class OVT_MapCampaignUIHandlerTypeIcon : Managed
{
	[Attribute()]
	string m_sType;
	
	[Attribute()]
	ResourceName m_Imageset;
	
	[Attribute()]
	string m_sIcon;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sName", true)]
class OVT_MapCampaignUIHandlerDefinition : Managed
{
	[Attribute("")]
	string m_sName;
	
	[Attribute("")]
	string m_sDataClassName;
	
	[Attribute("", UIWidgets.Object, "Handler Class (optional")]
	ref OVT_MapCampaignUIHandler m_Handler;
	
	[Attribute()]
	float m_fCeiling;
	
	[Attribute()]
	ResourceName m_Imageset;
	
	[Attribute()]
	string m_sIcon;
	
	[Attribute("", UIWidgets.Object, "Type Icons (optional")]
	ref array<ref OVT_MapCampaignUIHandlerTypeIcon> m_aTypeIcons;
	
	[Attribute("0")]
	bool m_bIsMilitaryInstallation;
}

class OVT_MapCampaignUIHandler : Managed
{
	protected ref OVT_MapLocationData m_Data;
	
	void SetData(OVT_MapLocationData data)
	{
		m_Data = data;
	}
	
	OVT_MapLocationData GetData()
	{
		return m_Data;
	}
	
	bool CanShow()
	{
		return true;
	}
	
	bool CanFastTravel()
	{
		return false;
	}
	
	bool CanRespawn()
	{
		return false;
	}
}


class OVT_MapCampaignUI : SCR_MapUIElementContainer
{
	[Attribute("{9B45BD0282167D90}UI/Layouts/Map/BaseElement.layout", params: "layout")]
	protected ResourceName m_sBaseElement;
	
	[Attribute("{B6846D46BDF6311E}UI/Layouts/Map/TownElement.layout", params: "layout")]
	protected ResourceName m_sTownElement;
	
	protected OVT_CampaignMapUIElement m_SelectedElement;
	protected OVT_MapInfoUI m_MapInfo;
	
	[Attribute("", UIWidgets.Object, "Overthrow map handlers")]
	protected ref array<ref OVT_MapCampaignUIHandlerDefinition> m_aHandlers;
	
	[Attribute("{F5E0CFFFC9F27B19}UI/Layouts/Map/MapIcon.layout")]
	protected ResourceName m_IconLayout;
	
	protected ref map<Widget, float> m_mCeilings = new map<Widget, float>();
	
	override void Init()
	{
		super.Init();
		
		OVT_MapInfoUI mapInfo = OVT_MapInfoUI.Cast(m_MapEntity.GetMapUIComponent(OVT_MapInfoUI));
		if (mapInfo)
		{
			m_MapInfo = mapInfo;
		}
	}
	
	protected void InitIcons()
	{
		OVT_EconomyManagerComponent economy = OVT_Global.GetEconomy();
		array<string> warned();
		
		foreach(OVT_MapLocationData location : economy.m_aMapLocations)
		{			
			string className = location.ClassName();
			
			OVT_MapCampaignUIHandlerDefinition definition = null;
			
			foreach(OVT_MapCampaignUIHandlerDefinition check : m_aHandlers)
			{
				if(check.m_sDataClassName == className)
				{
					definition = check;
					break;
				}
			}
			
			if(definition == null) {
				if(!warned.Contains(className))
				{
					warned.Insert(className);
					Print("[Overthrow.OVT_MapCampaignUI] No map handler definition found for " + className, LogLevel.WARNING);
				}
				continue;
			}
			
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_IconLayout, m_wIconsContainer);
			OVT_CampaignMapUIElement handler = OVT_CampaignMapUIElement.Cast(w.FindHandler(OVT_CampaignMapUIElement));
			
			if (!handler)
				return;
			
			handler.SetData(location);
						
			if(definition.m_bIsMilitaryInstallation)
			{
				handler.SetAsMilitaryInstallation();	
			}else{
				ResourceName imageset = definition.m_Imageset;
				string icon = definition.m_sIcon;
				
				foreach(OVT_MapCampaignUIHandlerTypeIcon def : definition.m_aTypeIcons)
				{
					if(def.m_sType == location.type)
					{
						imageset = def.m_Imageset;
						icon = def.m_sIcon;
					}
				}
				
				handler.SetIcon(imageset, icon);
			}			
			
			if(location.faction != -1)
			{
				handler.SetFaction(location.faction);
			}
			
			handler.GetOnMapIconClick().Insert(OnItemClick);			

			handler.SetParent(this);
			m_mIcons.Set(w, handler);
			m_mCeilings.Set(w, definition.m_fCeiling);

			FrameSlot.SetSizeToContent(w, true);
			FrameSlot.SetAlignment(w, 0.5, 0.5);
		}
	}
	
	protected void InitBases()
	{
		OVT_OccupyingFactionManager of = OVT_Global.GetOccupyingFaction();
		if(!of) return;
		
		foreach (OVT_BaseData baseData : of.m_Bases)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sBaseElement, m_wIconsContainer);
			OVT_CampaignMapUIBase handler = OVT_CampaignMapUIBase.Cast(w.FindHandler(OVT_CampaignMapUIBase));
			
			if (!handler)
				return;
			
			handler.GetOnMapIconClick().Insert(OnBaseClick);

			handler.SetParent(this);
			handler.InitBase(baseData);
			m_mIcons.Set(w, handler);

			FrameSlot.SetSizeToContent(w, true);
			FrameSlot.SetAlignment(w, 0.5, 0.5);
		}		
	}
	
	protected void InitTowns()
	{
		OVT_TownManagerComponent towns = OVT_Global.GetTowns();
		if(!towns) return;
		
		foreach(OVT_TownData town : towns.m_Towns)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sTownElement, m_wIconsContainer);
			OVT_CampaignMapUITown handler = OVT_CampaignMapUITown.Cast(w.FindHandler(OVT_CampaignMapUITown));
			
			if (!handler)
				return;
			
			handler.GetOnMapIconClick().Insert(OnTownClick);

			handler.SetParent(this);
			handler.InitTown(town);
			m_mIcons.Set(w, handler);

			FrameSlot.SetSizeToContent(w, true);
			FrameSlot.SetAlignment(w, 0.5, 0.5);
		}
	}
	
	protected void OnBaseClick(OVT_CampaignMapUIBase handler)
	{		
		if(m_SelectedElement) m_SelectedElement.DeselectIcon();
		handler.SelectIcon();
		m_SelectedElement = handler;
		
		if(m_MapInfo)
		{
			m_MapInfo.SelectBase(handler.GetBaseData());
		}
	}
	
	protected void OnTownClick(OVT_CampaignMapUITown handler)
	{		
		if(m_SelectedElement) m_SelectedElement.DeselectIcon();
		handler.SelectIcon();
		m_SelectedElement = handler;
		
		if(m_MapInfo)
		{
			m_MapInfo.SelectTown(handler.GetTownData());
		}
	}
	
	protected void OnItemClick(OVT_CampaignMapUIElement handler)
	{		
		if(m_SelectedElement) m_SelectedElement.DeselectIcon();
		handler.SelectIcon();
		m_SelectedElement = handler;
	}
	
	protected override void UpdateIcons()
	{
		foreach (Widget widget, SCR_MapUIElement icon : m_mIcons)
		{
			float c = m_mCeilings[widget];
			
			if(m_MapEntity.GetCurrentZoom() < c)
			{
				widget.SetOpacity(0);
			}else{
				widget.SetOpacity(1);
			}
		}
		
		super.UpdateIcons();
	}
	
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);

		InitIcons();
		//InitBases();
		//InitTowns();
		UpdateIcons();
	}
}