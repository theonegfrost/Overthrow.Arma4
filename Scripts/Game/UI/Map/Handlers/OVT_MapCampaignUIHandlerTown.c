[BaseContainerProps()]
class OVT_MapCampaignUIHandlerTown : OVT_MapCampaignUIHandler
{
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Modifier Layout", params: "layout")]
	ResourceName m_ModLayout;
	
	[Attribute(uiwidget: UIWidgets.ColorPicker)]
	ref Color m_NegativeModifierColor;
	
	[Attribute(uiwidget: UIWidgets.ColorPicker)]
	ref Color m_PositiveModifierColor;
	
	override void ShowInfo(Widget layoutRoot)
	{
		OVT_TownData town = OVT_TownData.Cast(m_Data);
		
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		
		OVT_TownManagerComponent townManager = OVT_Global.GetTowns();				
		int townID = town.id;
		
		ImageWidget img = ImageWidget.Cast(layoutRoot.FindAnyWidget("ControllingFaction"));
		img.LoadImageTexture(0, town.ControllingFactionData().GetUIInfo().GetIconPath());
				
		TextWidget widget = TextWidget.Cast(layoutRoot.FindAnyWidget("TownName"));
		widget.SetText(townManager.GetTownName(townID));
		
		widget = TextWidget.Cast(layoutRoot.FindAnyWidget("Population"));
		widget.SetText(town.population.ToString());
		
		widget = TextWidget.Cast(layoutRoot.FindAnyWidget("Distance"));
		float distance = vector.Distance(town.location, player.GetOrigin());
		string dis, units;
		SCR_Global.GetDistForHUD(distance, false, dis, units);
		widget.SetText(dis + " " + units);
		
		widget = TextWidget.Cast(layoutRoot.FindAnyWidget("Stability"));
		widget.SetText(town.stability.ToString() + "%");
		
		widget = TextWidget.Cast(layoutRoot.FindAnyWidget("Support"));		
		widget.SetText(town.support.ToString() + " (" + town.SupportPercentage().ToString() + "%)");
		
		Widget container = layoutRoot.FindAnyWidget("StabilityModContainer");
		Widget child = container.GetChildren();
		while(child)
		{
			container.RemoveChild(child);
			child = container.GetChildren();
		}
		autoptr array<int> done = new array<int>;
		OVT_TownModifierSystem system = townManager.GetModifierSystem(OVT_TownStabilityModifierSystem);
		WorkspaceWidget workspace = GetGame().GetWorkspace(); 
		foreach(OVT_TownModifierData data : town.stabilityModifiers)
		{
			if(done.Contains(data.id)) continue;
			
			OVT_ModifierConfig mod = system.m_Config.m_aModifiers[data.id];
			
			Widget w = workspace.CreateWidgets(m_ModLayout, container);
			TextWidget tw = TextWidget.Cast(w.FindAnyWidget("Text"));
			
			int effect = mod.baseEffect;
			if(mod.flags & OVT_ModifierFlags.STACKABLE)
			{
				effect = 0;
				//count all present
				foreach(OVT_TownModifierData check : town.stabilityModifiers)
				{
					if(check.id == data.id) effect += mod.baseEffect;
				}
			}
			
			tw.SetText(effect.ToString() + "% " + mod.title);
			
			PanelWidget panel = PanelWidget.Cast(w.FindAnyWidget("Background"));
			if(mod.baseEffect < 0)
			{
				panel.SetColor(m_NegativeModifierColor);
			}else{
				panel.SetColor(m_PositiveModifierColor);
			}
			done.Insert(data.id);
		}
		
		container = layoutRoot.FindAnyWidget("SupportModContainer");
		child = container.GetChildren();
		while(child)
		{
			container.RemoveChild(child);
			child = container.GetChildren();
		}
		done.Clear();
		
		system = townManager.GetModifierSystem(OVT_TownSupportModifierSystem);
		foreach(OVT_TownModifierData data : town.supportModifiers)
		{
			if(done.Contains(data.id)) continue;
			OVT_ModifierConfig mod = system.m_Config.m_aModifiers[data.id];
			Widget w = workspace.CreateWidgets(m_ModLayout, container);
			TextWidget tw = TextWidget.Cast(w.FindAnyWidget("Text"));
			int effect = mod.baseEffect;
			if(mod.flags & OVT_ModifierFlags.STACKABLE)
			{
				effect = 0;
				//count all present
				foreach(OVT_TownModifierData check : town.supportModifiers)
				{
					if(check.id == data.id) effect += mod.baseEffect;
				}
			}
			
			tw.SetText(effect.ToString() + "% " + mod.title);
			
			PanelWidget panel = PanelWidget.Cast(w.FindAnyWidget("Background"));
			if(mod.baseEffect < 0)
			{
				panel.SetColor(m_NegativeModifierColor);
			}else{
				panel.SetColor(m_PositiveModifierColor);
			}
			done.Insert(data.id);
		}
	}
}