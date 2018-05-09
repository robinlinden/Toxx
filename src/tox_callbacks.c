#include "tox_callbacks.h"

#include "avatar.h"
#include "file_transfers.h"
#include "friend.h"
#include "groups.h"
#include "macros.h"
#include "settings.h"
#include "text.h"
#include "utox.h"
#include "ui.h"

#include "av/audio.h"
#include "av/utox_av.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void callback_friend_request(Tox *UNUSED(tox), const uint8_t *id, const uint8_t *msg,
                                    size_t length, void *UNUSED(userdata)) {
    length = utf8_validate(msg, length);

    uint16_t r_number = friend_request_new(id, msg, length);

    postmessage_utox(FRIEND_INCOMING_REQUEST, r_number, 0, NULL);
    postmessage_audio(UTOXAUDIO_PLAY_NOTIFICATION, NOTIFY_TONE_FRIEND_REQUEST, 0, NULL);
}

static void callback_friend_message(Tox *UNUSED(tox), uint32_t friend_number, TOX_MESSAGE_TYPE type,
                                    const uint8_t *message, size_t length, void *UNUSED(userdata)) {
    /* send message to UI */
    FRIEND *f = get_friend(friend_number);
    if (!f) {
        return;
    }

    switch (type) {
        case TOX_MESSAGE_TYPE_NORMAL: {
            message_add_type_text(&f->msg, 0, (char *)message, length, 1, 0);
            break;
        }

        case TOX_MESSAGE_TYPE_ACTION: {
            message_add_type_action(&f->msg, 0, (char *)message, length, 1, 0);
            break;
        }

        default: {
            break;
        }
    }
    friend_notify_msg(f, (char *)message, length);
}

static void callback_name_change(Tox *UNUSED(tox), uint32_t fid, const uint8_t *newname, size_t length,
                                 void *UNUSED(userdata))
{
    length     = utf8_validate(newname, length);
    void *data = malloc(length);
    memcpy(data, newname, length);
    postmessage_utox(FRIEND_NAME, fid, length, data);
}

static void callback_status_message(Tox *UNUSED(tox), uint32_t fid, const uint8_t *newstatus, size_t length,
                                    void *UNUSED(userdata))
{
    length     = utf8_validate(newstatus, length);
    void *data = malloc(length);
    memcpy(data, newstatus, length);
    postmessage_utox(FRIEND_STATUS_MESSAGE, fid, length, data);
}

static void callback_user_status(Tox *UNUSED(tox), uint32_t fid, TOX_USER_STATUS status,
                                 void *UNUSED(userdata))
{
    postmessage_utox(FRIEND_STATE, fid, status, NULL);
}

static void callback_typing_change(Tox *UNUSED(tox), uint32_t fid, bool is_typing, void *UNUSED(userdata)) {
    postmessage_utox(FRIEND_TYPING, fid, is_typing, NULL);
}

static void callback_read_receipt(Tox *UNUSED(tox), uint32_t fid, uint32_t receipt, void *UNUSED(userdata)) {
    FRIEND *f = get_friend(fid);
    if (!f) {
        return;
    }

    messages_clear_receipt(&f->msg, receipt);
}

static void callback_connection_status(Tox *tox, uint32_t fid, TOX_CONNECTION status, void *UNUSED(userdata)) {
    FRIEND *f = get_friend(fid);
    if (!f) {
        return;
    }

    if (f->online && !status) {
        ft_friend_offline(tox, fid);
        if (f->call_state_self || f->call_state_friend) {
            utox_av_local_disconnect(NULL, fid); /* TODO HACK, toxav doesn't supply a toxav_get_toxav_from_tox() yet. */
        }
    } else if (!f->online && !!status) {
        ft_friend_online(tox, fid);
        /* resend avatar info (in case it changed) */
        /* Avatars must be sent LAST or they will clobber existing file transfers! */
        avatar_on_friend_online(tox, fid);
        friend_notify_status(f, (uint8_t *)f->status_message, f->status_length, "online");
    }
    postmessage_utox(FRIEND_ONLINE, fid, !!status, NULL);

    if (status == TOX_CONNECTION_UDP) {
    } else if (status == TOX_CONNECTION_TCP) {
    } else {
        friend_notify_status(f, NULL, 0, "offline");
    }
}

void utox_set_callbacks_friends(Tox *tox) {
    tox_callback_friend_request(tox, callback_friend_request);
    tox_callback_friend_message(tox, callback_friend_message);
    tox_callback_friend_name(tox, callback_name_change);
    tox_callback_friend_status_message(tox, callback_status_message);
    tox_callback_friend_status(tox, callback_user_status);
    tox_callback_friend_typing(tox, callback_typing_change);
    tox_callback_friend_read_receipt(tox, callback_read_receipt);
    tox_callback_friend_connection_status(tox, callback_connection_status);
}

static void callback_group_invite(Tox *tox, uint32_t fid, TOX_CONFERENCE_TYPE type, const uint8_t *data, size_t length,
                                  void *UNUSED(userdata))
{
    uint32_t gid = UINT32_MAX;
    if (type == TOX_CONFERENCE_TYPE_TEXT) {
        gid = tox_conference_join(tox, fid, data, length, NULL);
    } else if (type == TOX_CONFERENCE_TYPE_AV) {
        gid = toxav_join_av_groupchat(tox, fid, data, length, callback_av_group_audio, NULL);
    }

    if (gid == UINT32_MAX) {
        return;
    }

    GROUPCHAT *g = get_group(gid);
    if (!g) {
        group_create(gid, type == TOX_CONFERENCE_TYPE_AV ? true : false);
    } else {
        group_init(g, gid, type == TOX_CONFERENCE_TYPE_AV ? true : false);
    }

    postmessage_utox(GROUP_ADD, gid, 0, tox);
}

static void callback_group_message(Tox *UNUSED(tox), uint32_t gid, uint32_t pid, TOX_MESSAGE_TYPE type,
                                   const uint8_t *message, size_t length, void *UNUSED(userdata))
{
    GROUPCHAT *g = get_group(gid);

    switch (type) {
        case TOX_MESSAGE_TYPE_ACTION: {
            group_add_message(g, pid, message, length, MSG_TYPE_ACTION_TEXT);
            break;
        }
        case TOX_MESSAGE_TYPE_NORMAL: {
            group_add_message(g, pid, message, length, MSG_TYPE_TEXT);
            break;
        }
    }
    group_notify_msg(g, (const char *)message, length);
    postmessage_utox(GROUP_MESSAGE, gid, pid, NULL);
}

static void callback_group_peer_name_change(
        Tox *UNUSED(tox),
        uint32_t gid,
        uint32_t pid,
        const uint8_t *name,
        size_t length,
        void *UNUSED(userdata))
{
    GROUPCHAT *g = get_group(gid);
    if (!g) {
        assert(false);
        return;
    }

    length = utf8_validate(name, length);
    group_peer_name_change(g, pid, name, length);

    postmessage_utox(GROUP_PEER_NAME, gid, pid, NULL);
}

static void callback_group_peer_list_changed(Tox *tox, uint32_t gid, void *UNUSED(userdata)){
    GROUPCHAT *g = get_group(gid);
    if (!g) {
        assert(false);
        return;
    }

    pthread_mutex_lock(&messages_lock);
    group_reset_peerlist(g);

    uint32_t number_peers = tox_conference_peer_count(tox, gid, NULL);

    g->peer = calloc(number_peers, sizeof(void *));
    g->peer_count = number_peers;

    for (uint32_t i = 0; i < number_peers; ++i) {
        uint8_t tmp[TOX_MAX_NAME_LENGTH];
        size_t len = tox_conference_peer_get_name_size(tox, gid, i, NULL);
        tox_conference_peer_get_name(tox, gid, i, tmp, NULL);

        GROUP_PEER *peer = calloc(1, sizeof(*peer) + len + 1);
        memcpy(peer->name, tmp, len);
        peer->name_length = len;

        peer->id = i;

        /* get static random color */
        uint8_t pkey[TOX_PUBLIC_KEY_SIZE];
        tox_conference_peer_get_public_key(tox, gid, i, pkey, NULL);
        uint64_t pkey_to_number = 0;
        for (int key_i = 0; key_i < TOX_PUBLIC_KEY_SIZE; ++key_i) {
            pkey_to_number += pkey[key_i];
        }
        srand(pkey_to_number);
        peer->name_color = RGB(rand(), rand(), rand());

        g->peer[i] = peer;
    }

    postmessage_utox(GROUP_PEER_CHANGE, gid, 0, NULL);
    pthread_mutex_unlock(&messages_lock);
}

static void callback_group_topic(Tox *UNUSED(tox), uint32_t gid, uint32_t UNUSED(pid),
                                 const uint8_t *title, size_t length, void *UNUSED(userdata))
{
    length = utf8_validate(title, length);
    if (!length) {
        return;
    }

    uint8_t *copy_title = malloc(length);
    memcpy(copy_title, title, length);
    postmessage_utox(GROUP_TOPIC, gid, length, copy_title);
}

void utox_set_callbacks_groups(Tox *tox) {
    tox_callback_conference_invite(tox, callback_group_invite);
    tox_callback_conference_message(tox, callback_group_message);
    tox_callback_conference_peer_name(tox, callback_group_peer_name_change);
    tox_callback_conference_title(tox, callback_group_topic);
    tox_callback_conference_peer_list_changed(tox, callback_group_peer_list_changed);
}
