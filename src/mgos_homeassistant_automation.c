/*
 * Copyright 2020 Pim van Pelt <pim@ipng.nl>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mgos_homeassistant_automation.h"

#include <strings.h>

#include "mgos.h"

struct mgos_homeassistant_automation *mgos_homeassistant_automation_create(
    void *user_data) {
  struct mgos_homeassistant_automation *a = calloc(1, sizeof(*a));
  if (!a) return NULL;

  a->user_data = user_data;
  SLIST_INIT(&a->triggers);
  SLIST_INIT(&a->conditions);
  SLIST_INIT(&a->actions);
  LOG(LL_DEBUG, ("Created automation"));
  return a;
}

bool mgos_homeassistant_automation_set_trigger_cb(
    struct mgos_homeassistant_automation *a,
    mgos_homeassistant_automation_cb cb) {
  if (!a) return false;
  a->trigger_cb = cb;
  return true;
}

bool mgos_homeassistant_automation_set_condition_cb(
    struct mgos_homeassistant_automation *a,
    mgos_homeassistant_automation_cb cb) {
  if (!a) return false;
  a->condition_cb = cb;
  return true;
}

bool mgos_homeassistant_automation_set_action_cb(
    struct mgos_homeassistant_automation *a,
    mgos_homeassistant_automation_cb cb) {
  if (!a) return false;
  a->action_cb = cb;
  return true;
}

bool mgos_homeassistant_automation_add_trigger(
    struct mgos_homeassistant_automation *a,
    enum mgos_homeassistant_automation_datatype type, void *data) {
  if (!a) return false;
  struct mgos_homeassistant_automation_data *d =
      mgos_homeassistant_automation_data_create(type, data);
  if (!d) return false;
  SLIST_INSERT_HEAD(&a->triggers, d, entry);
  LOG(LL_DEBUG, ("Inserted automation trigger data type %d", type));
  return true;
}

bool mgos_homeassistant_automation_add_condition(
    struct mgos_homeassistant_automation *a,
    enum mgos_homeassistant_automation_datatype type, void *data) {
  if (!a) return false;
  struct mgos_homeassistant_automation_data *d =
      mgos_homeassistant_automation_data_create(type, data);
  if (!d) return false;
  SLIST_INSERT_HEAD(&a->conditions, d, entry);
  LOG(LL_DEBUG, ("Inserted automation condition data type %d", type));
  return true;
}

bool mgos_homeassistant_automation_add_action(
    struct mgos_homeassistant_automation *a,
    enum mgos_homeassistant_automation_datatype type, void *data) {
  if (!a) return false;
  struct mgos_homeassistant_automation_data *d =
      mgos_homeassistant_automation_data_create(type, data);
  if (!d) return false;
  SLIST_INSERT_HEAD(&a->actions, d, entry);
  LOG(LL_DEBUG, ("Inserted automation action data type %d", type));
  return true;
}

bool mgos_homeassistant_automation_fromfile(
    struct mgos_homeassistant_automation *a, const char *filename) {
  return mgos_homeassistant_automation_fromjson(a, json_fread(filename));
}

bool mgos_homeassistant_automation_fromjson(
    struct mgos_homeassistant_automation *a, const char *json) {
  struct json_token val;
  void *h = NULL;
  int idx;

  if (!a || !json) return false;
  while ((h = json_next_elem(json, strlen(json), h, ".trigger", &idx, &val)) !=
         NULL) {
    LOG(LL_DEBUG, ("idx[%d] '%.*s'", idx, (int) val.len, val.ptr));
    char *j_type = NULL;
    json_scanf(val.ptr, val.len, "{type:%Q}", &j_type);
    if (!j_type || 0 == strcasecmp(j_type, "status")) {
      struct mgos_homeassistant_automation_data_status *dd =
          calloc(1, sizeof(*dd));
      if (dd) {
        json_scanf(val.ptr, val.len, "{object:%Q,status:%Q}", &dd->object,
                   &dd->status);
        if (!mgos_homeassistant_automation_add_trigger(a, TRIGGER_STATUS, dd)) {
          LOG(LL_WARN, ("Could not add trigger JSON: %.*s, skipping",
                        (int) val.len, val.ptr));
        } else {
          LOG(LL_DEBUG, ("Added trigger JSON: %.*s", (int) val.len, val.ptr));
        }
      }
    } else {
      LOG(LL_WARN, ("Unknown data type '%s', skipping ..", j_type));
    }
    if (j_type) free(j_type);
  }
  while ((h = json_next_elem(json, strlen(json), h, ".condition", &idx,
                             &val)) != NULL) {
    LOG(LL_DEBUG, ("idx[%d] '%.*s'", idx, (int) val.len, val.ptr));
    char *j_type = NULL;
    json_scanf(val.ptr, val.len, "{type:%Q}", &j_type);
    if (!j_type || 0 == strcasecmp(j_type, "status")) {
      struct mgos_homeassistant_automation_data_status *dd =
          calloc(1, sizeof(*dd));
      if (dd) {
        json_scanf(val.ptr, val.len, "{object:%Q,status:%Q}", &dd->object,
                   &dd->status);
        if (!mgos_homeassistant_automation_add_condition(a, CONDITION_STATUS,
                                                         dd)) {
          LOG(LL_WARN, ("Could not add condition JSON: %.*s, skipping",
                        (int) val.len, val.ptr));
        } else {
          LOG(LL_DEBUG, ("Added condition JSON: %.*s", (int) val.len, val.ptr));
        }
      }
    } else {
      LOG(LL_WARN, ("Unknown data type '%s', skipping ..", j_type));
    }
    if (j_type) free(j_type);
  }
  while ((h = json_next_elem(json, strlen(json), h, ".action", &idx, &val)) !=
         NULL) {
    LOG(LL_DEBUG, ("idx[%d] '%.*s'", idx, (int) val.len, val.ptr));
    char *j_type = NULL;
    json_scanf(val.ptr, val.len, "{type:%Q}", &j_type);
    if (!j_type || 0 == strcasecmp(j_type, "mqtt")) {
      struct mgos_homeassistant_automation_data_action_mqtt *dd =
          calloc(1, sizeof(*dd));
      if (dd) {
        json_scanf(val.ptr, val.len, "{topic:%Q,payload:%Q}", &dd->topic,
                   &dd->payload);
        if (!mgos_homeassistant_automation_add_action(a, ACTION_MQTT, dd)) {
          LOG(LL_WARN, ("Could not add action JSON: %.*s, skipping",
                        (int) val.len, val.ptr));
        } else {
          LOG(LL_DEBUG, ("Added action JSON: %.*s", (int) val.len, val.ptr));
        }
      }
    } else if (0 == strcasecmp(j_type, "command")) {
      struct mgos_homeassistant_automation_data_action_command *dd =
          calloc(1, sizeof(*dd));
      if (dd) {
        json_scanf(val.ptr, val.len, "{object:%Q,payload:%Q}", &dd->object,
                   &dd->payload);
        if (!mgos_homeassistant_automation_add_action(a, ACTION_COMMAND, dd)) {
          LOG(LL_WARN, ("Could not add action JSON: %.*s, skipping",
                        (int) val.len, val.ptr));
        } else {
          LOG(LL_DEBUG, ("Added action JSON: %.*s", (int) val.len, val.ptr));
        }
      }
    } else {
      LOG(LL_WARN, ("Unknown data type '%s', skipping ..", j_type));
    }
    if (j_type) free(j_type);
  }

  return true;
}

bool mgos_homeassistant_automation_run(
    struct mgos_homeassistant_automation *a,
    enum mgos_homeassistant_automation_datatype trigger_type,
    void *trigger_data) {
  return false;
  (void) a;
  (void) trigger_type;
  (void) trigger_data;
}

struct mgos_homeassistant_automation_data *
mgos_homeassistant_automation_data_create(
    enum mgos_homeassistant_automation_datatype type, void *data) {
  struct mgos_homeassistant_automation_data *d = calloc(1, sizeof(*d));
  if (!d) return NULL;
  LOG(LL_DEBUG, ("Creating automation data type %d", type));
  d->type = type;
  d->data = data;
  return d;
}

bool mgos_homeassistant_automation_data_destroy(
    struct mgos_homeassistant_automation_data **d) {
  if (!(*d)) return false;
  if (!(*d)->data) return true;
  LOG(LL_DEBUG, ("Destroying automation data type %d", (*d)->type));

  switch ((*d)->type) {
    case TRIGGER_STATUS:
    case CONDITION_STATUS: {
      struct mgos_homeassistant_automation_data_status *dd = (*d)->data;
      if (!dd) return true;
      if (dd->object) free(dd->object);
      if (dd->status) free(dd->status);
      break;
    }
    case ACTION_MQTT: {
      struct mgos_homeassistant_automation_data_action_mqtt *dd = (*d)->data;
      if (!dd) return true;
      if (dd->topic) free(dd->topic);
      if (dd->payload) free(dd->payload);
      break;
    }
    case ACTION_COMMAND: {
      struct mgos_homeassistant_automation_data_action_command *dd = (*d)->data;
      if (!dd) return true;
      if (dd->object) free(dd->object);
      if (dd->payload) free(dd->payload);
      break;
    }
    default:
      LOG(LL_WARN,
          ("Automation data type %d unknown, skipping .. ", (*d)->type));
  }
  return true;
}

bool mgos_homeassistant_automation_destroy(
    struct mgos_homeassistant_automation **a) {
  if (!(*a)) return false;
  LOG(LL_DEBUG, ("Destroying automation"));

  while (!SLIST_EMPTY(&(*a)->triggers)) {
    struct mgos_homeassistant_automation_data *d;
    d = SLIST_FIRST(&(*a)->triggers);
    mgos_homeassistant_automation_data_destroy(&d);
  }
  while (!SLIST_EMPTY(&(*a)->conditions)) {
    struct mgos_homeassistant_automation_data *d;
    d = SLIST_FIRST(&(*a)->conditions);
    mgos_homeassistant_automation_data_destroy(&d);
  }
  while (!SLIST_EMPTY(&(*a)->actions)) {
    struct mgos_homeassistant_automation_data *d;
    d = SLIST_FIRST(&(*a)->actions);
    mgos_homeassistant_automation_data_destroy(&d);
  }

  free(*a);
  *a = NULL;
  return true;
}
