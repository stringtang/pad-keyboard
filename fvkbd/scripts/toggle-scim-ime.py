#!/usr/bin/python

import sys
import dbus

try:
	bus = dbus.SessionBus()
	if(bus):
		obj = bus.get_object("org.moblin.scim.vkb", "/org/moblin/scim/vkb")
		if(obj):
			client = dbus.Interface(obj,"org.moblin.scim.vkb")
			client.ToggleVkbAlone()
except:
	pass
