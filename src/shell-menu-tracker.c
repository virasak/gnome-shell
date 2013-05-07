/*
 * Copyright (C) 2013 Red Hat
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by:
 *     Jasper St. Pierre <jstpierre@mecheye.net>
 */

#include "config.h"

#include "shell-menu-tracker.h"
#include "gtkmenutracker.h"

struct _ShellMenuTracker
{
  guint ref_count;

  GtkMenuTracker *tracker;

  ShellMenuTrackerInsertFunc insert_func;
  gpointer insert_user_data;
  GDestroyNotify insert_notify;
  ShellMenuTrackerRemoveFunc remove_func;
  gpointer remove_user_data;
  GDestroyNotify remove_notify;
};

static void
shell_menu_tracker_insert_func (gint position,
                                GMenuModel *model,
                                gint item_index,
                                const gchar *action_namespace,
                                gboolean is_separator,
                                gpointer user_data)
{
  ShellMenuTracker *tracker = (ShellMenuTracker *) user_data;
  tracker->insert_func (position, model, item_index,
                        action_namespace, is_separator,
                        tracker->insert_user_data);
}

static void
shell_menu_tracker_remove_func (gint position,
                                gpointer user_data)
{
  ShellMenuTracker *tracker = (ShellMenuTracker *) user_data;
  tracker->remove_func (position, tracker->remove_user_data);
}

/**
 * shell_menu_tracker_new:
 * @model:
 * @action_namespace: (allow-none):
 * @insert_func:
 * @insert_user_data:
 * @insert_notify:
 * @remove_func:
 * @remove_user_data:
 * @remove_notify:
 */
ShellMenuTracker *
shell_menu_tracker_new (GMenuModel                 *model,
                        const gchar                *action_namespace,
                        ShellMenuTrackerInsertFunc  insert_func,
                        gpointer                    insert_user_data,
                        GDestroyNotify              insert_notify,
                        ShellMenuTrackerRemoveFunc  remove_func,
                        gpointer                    remove_user_data,
                        GDestroyNotify              remove_notify)
{
  ShellMenuTracker *tracker = g_slice_new0 (ShellMenuTracker);

  tracker->ref_count = 1;
  tracker->insert_func = insert_func;
  tracker->insert_user_data = insert_user_data;
  tracker->insert_notify = insert_notify;
  tracker->remove_func = remove_func;
  tracker->remove_user_data = remove_user_data;
  tracker->remove_notify = remove_notify;

  tracker->tracker = gtk_menu_tracker_new (model,
                                           TRUE, /* with separators */
                                           action_namespace,
                                           shell_menu_tracker_insert_func,
                                           shell_menu_tracker_remove_func,
                                           tracker);

  return tracker;
}

ShellMenuTracker *
shell_menu_tracker_ref (ShellMenuTracker *tracker)
{
  tracker->ref_count++;
  return tracker;
}

void
shell_menu_tracker_unref (ShellMenuTracker *tracker)
{
  if (tracker->ref_count-- <= 0)
    {
      shell_menu_tracker_destroy (tracker);
      g_slice_free (ShellMenuTracker, tracker);
    }
}

void
shell_menu_tracker_destroy (ShellMenuTracker *tracker)
{
  if (tracker->tracker != NULL)
    {
      gtk_menu_tracker_free (tracker->tracker);
      tracker->tracker = NULL;
      tracker->insert_notify (tracker->insert_user_data);
      tracker->remove_notify (tracker->remove_user_data);
    }
}

G_DEFINE_BOXED_TYPE(ShellMenuTracker,
                    shell_menu_tracker,
                    shell_menu_tracker_ref,
                    shell_menu_tracker_unref)