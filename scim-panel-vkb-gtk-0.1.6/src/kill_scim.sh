#!/bin/sh

ps -ef | grep scim | grep -v grep | cut -c 9-15 | xargs kill -9
