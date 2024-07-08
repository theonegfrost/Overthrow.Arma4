class OVT_CampaignMapUIElement : SCR_MapUIElement
{
	protected string m_sFactionKey;
	protected Widget m_wBaseIcon;
	protected Widget m_wIcon;
	protected SCR_MilitarySymbolUIComponent m_wSymbolUI;
	
	protected ref ScriptInvoker m_OnMapIconEnter;
	protected ref ScriptInvoker m_OnMapIconClick;
	protected ref ScriptInvoker m_OnMapIconSelected;
	
	protected ref OVT_MapLocationData m_Data;
	protected bool m_bIsMilitaryInstallation;
	
	void SetData(OVT_MapLocationData data)
	{
		m_Data = data;
	}
	
	void SetAsMilitaryInstallation()
	{
		m_bIsMilitaryInstallation = true;	
		m_wIcon.SetVisible(false);
		m_wBaseIcon.SetVisible(true);	
	}
	
	void SetIcon(ResourceName imageset, string icon)
	{
		if (!m_wBaseIcon)
			return;
		
		ImageWidget image = ImageWidget.Cast(m_wIcon.FindAnyWidget("Icon"));
		image.LoadImageFromSet(0, imageset, icon);
	}
	
	ScriptInvoker GetOnMapIconEnter()
	{
		if (!m_OnMapIconEnter)
			m_OnMapIconEnter = new ScriptInvoker();

		return m_OnMapIconEnter;
	}
	
	ScriptInvoker GetOnMapIconClick()
	{
		if (!m_OnMapIconClick)
			m_OnMapIconClick = new ScriptInvoker();

		return m_OnMapIconClick;
	}
	
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (m_OnMapIconClick)
			m_OnMapIconClick.Invoke(this);

		return false;
	}
	
	override void SelectIcon(bool invoke=true)
	{
		m_wSelectImg.SetVisible(true);
	}
	
	void DeselectIcon()
	{
		m_wSelectImg.SetVisible(false);
	}
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		SCR_UITaskManagerComponent tm = SCR_UITaskManagerComponent.GetInstance();
		if (tm && !tm.IsTaskListOpen())
		{
			GetGame().GetWorkspace().SetFocusedWidget(w);
		}

		if (m_OnMapIconEnter)
			m_OnMapIconEnter.Invoke();

		super.OnMouseEnter(w, x, y);
		
		m_wHighlightImg.SetVisible(true);

		return false;
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		m_wHighlightImg.SetVisible(false);
		return false;
	}
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wBaseIcon = w.FindAnyWidget("SideSymbol");	
		m_wIcon = w.FindAnyWidget("IconSymbol");	
		m_wSymbolUI = SCR_MilitarySymbolUIComponent.Cast(m_wBaseIcon.FindHandler(SCR_MilitarySymbolUIComponent));					
	}
	
	protected void SetIconFaction(Faction faction)
	{
		if(!faction) return;
		m_sFactionKey = faction.GetFactionKey();
		SetBaseIconFactionColor(faction);
	}
	
	void SetFaction(int faction)
	{
		FactionManager fm = GetGame().GetFactionManager();
		Faction fac = fm.GetFactionByIndex(faction);
		m_sFactionKey = fac.GetFactionKey();
		SetBaseIconFactionColor(fac);
		
		if(!m_wSymbolUI) return;
			
		SCR_MilitarySymbol baseIcon = new SCR_MilitarySymbol();
			
		if(m_bIsMilitaryInstallation)
		{
			if(m_Data.IsOccupyingFaction())
			{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.OPFOR);
			}else{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.BLUFOR);
			}
			
			baseIcon.SetDimension(EMilitarySymbolDimension.INSTALLATION);			
		}else{
			baseIcon.SetDimension(EMilitarySymbolDimension.NONE);
		}
		m_wSymbolUI.Update(baseIcon);
	}
	
	string GetFactionKey()
	{
		return m_sFactionKey;
	}
	//------------------------------------------------------------------------------------------------
	Color GetFactionColor()
	{
		return GetColorForFaction(m_sFactionKey);
	}
		
	void SetBaseIconFactionColor(Faction faction)
	{
		if (!m_wBaseIcon)
			return;

		Color color;
		
		if (faction)
			color = faction.GetFactionColor();
		else
			color = GetColorForFaction("");

		m_wBaseIcon.SetColor(color);
		if (m_wGradient)
			m_wGradient.SetColor(color);
		
		if (!m_wIcon)
			return;
		
		m_wIcon.SetColor(color);
	}
	
	override vector GetPos()
	{		
		return m_Data.location;
	}
}