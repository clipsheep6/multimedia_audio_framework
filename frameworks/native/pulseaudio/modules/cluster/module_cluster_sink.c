/***
  This file is part of PulseAudio.

  Copyright 2004-2008 Lennart Poettering

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2.1 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, see <http://www.gnu.org/licenses/>.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pulsecore/sink.h>
#include <pulsecore/module.h>
#include <pulsecore/core-util.h>
#include <pulsecore/modargs.h>
#include <pulsecore/log.h>
#include <pulsecore/namereg.h>

#include "audio_log.h"

PA_MODULE_AUTHOR("OpenHarmony");
PA_MODULE_DESCRIPTION(_("Cluster module"));
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(false);
PA_MODULE_USAGE(
        "sink_name=<name of sink> "
);

struct userdata {
    pa_core *core;
    pa_module *module;
};

static const char* const valid_modargs[] = {
    "sink_name",
    NULL
};

static void ShowNumInputForEachSink(pa_core *c)
{
    pa_sink *sink;
    pa_sink_input *si;
    uint32_t idx, numInputs;

    PA_IDXSET_FOREACH(sink, c->sinks, idx) {
        pa_proplist_sets(sink->proplist, PA_PROP_DEVICE_BUS, "0");
    }

    PA_IDXSET_FOREACH(si, c->sink_inputs, idx) {
        const char *sceneType = pa_proplist_gets(si->proplist, "scene.type");
        if (pa_safe_streq(sceneType, "N/A")) {
            continue; // if sinkInput is created by ourselves, skip
        }
        const char *numInputsStr = pa_proplist_gets(si->sink->proplist, PA_PROP_DEVICE_BUS);
        pa_atou(numInputsStr, &numInputs);
        numInputs++;
        char newNumInputsStr[50];
        sprintf(newNumInputsStr, "%u", numInputs);
        pa_proplist_sets(si->sink->proplist, PA_PROP_DEVICE_BUS, newNumInputsStr);
    }
}

static pa_hook_result_t SinkInputChangedCb(pa_core *c, pa_sink_input *si, struct userdata *u)
{
    ShowNumInputForEachSink(c);
    return PA_HOOK_OK;
}

static pa_hook_result_t SinkInputProplistChangedCb(pa_core *c, pa_sink_input *si, struct userdata *u)
{
    pa_sink *effectSink;
    pa_assert(c);
    pa_assert(u);
    const char *sceneMode = pa_proplist_gets(si->proplist, "scene.mode");
    const char *sceneType = pa_proplist_gets(si->proplist, "scene.type");

    // check default/none
    if (pa_safe_streq(sceneMode, "EFFECT_NONE")) {
        pa_sink_input_move_to(si, c->default_sink, false); //if bypass move to hdi sink
        // For Testing
        ShowNumInputForEachSink(c);
        return PA_HOOK_OK;
    }

    effectSink = pa_namereg_get(c, sceneType, PA_NAMEREG_SINK);
    if (!effectSink) { // if sink does not exist
        AUDIO_ERR_LOG("Effect sink [%{public}s] sink not found.", sceneType);
        // classify sinkinput to default sink
        pa_sink_input_move_to(si, c->default_sink, false);
    } else {
        // classify sinkinput to effect sink
        pa_sink_input_move_to(si, effectSink, false);
    }

    // For Testing
    ShowNumInputForEachSink(c);
    return PA_HOOK_OK;
}

int InitFail(pa_module *m, pa_modargs *ma)
{
    AUDIO_ERR_LOG("Failed to create cluster module");
    if (ma)
        pa_modargs_free(ma);
    pa__done(m);
    return -1;
}

int pa__init(pa_module *m)
{
    struct userdata *u = NULL;
    pa_modargs *ma = NULL;

    pa_assert(m);

    if (!(ma = pa_modargs_new(m->argument, valid_modargs))) {
        AUDIO_ERR_LOG("Failed to parse module arguments.");
        return InitFail(m, ma);
    }

    m->userdata = u = pa_xnew0(struct userdata, 1);
    u->core = m->core;
    u->module = m;
    
    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SINK_INPUT_PROPLIST_CHANGED],
                           PA_HOOK_LATE, (pa_hook_cb_t) SinkInputProplistChangedCb, u);
	// For Testing BEGIN
    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SINK_INPUT_UNLINK],
                           PA_HOOK_EARLY, (pa_hook_cb_t) SinkInputChangedCb, u);
    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SINK_INPUT_UNLINK_POST],
                           PA_HOOK_EARLY, (pa_hook_cb_t) SinkInputChangedCb, u);
    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SINK_INPUT_STATE_CHANGED],
                           PA_HOOK_EARLY, (pa_hook_cb_t) SinkInputChangedCb, u);
	// For Testing END

    pa_modargs_free(ma);

    return 0;
}

int pa__get_n_used(pa_module *m)
{
    struct userdata *u;

    pa_assert(m);
    pa_assert_se(u = m->userdata);

    return 0;
}

void pa__done(pa_module *m)
{
    struct userdata *u;

    pa_assert(m);

    if (!(u = m->userdata)) {
        return;
    }

    pa_xfree(u);
}
