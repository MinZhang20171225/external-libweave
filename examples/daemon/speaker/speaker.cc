// Copyright 2015 The Weave Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/daemon/common/daemon.h"

#include <weave/device.h>

#include <base/bind.h>
#include <base/memory/weak_ptr.h>

// SpeakerHandler is a command handler example that shows
// how to handle commands for a Weave speaker.
class SpeakerHandler {
 public:
  SpeakerHandler() = default;
  void Register(weave::Device* device) {
    device_ = device;

    device->AddStateDefinitionsFromJson(R"({
      "onOff": {"state": {"type": "string", "enum": ["on", "standby"]}},
      "volume": {
        "volume": {"type": "integer"},
        "isMuted": {"type": "boolean"}
      }
    })");

    device->SetStatePropertiesFromJson(R"({
      "onOff":{"state": "standby"},
      "volume":{
        "volume": 100,
        "isMuted": false
      }
    })",
                                       nullptr);

    device->AddCommandDefinitionsFromJson(R"({
      "onOff": {
         "setConfig":{
           "parameters": {
             "state": {"type": "string", "enum": ["on", "standby"]}
           }
         }
       },
       "volume": {
         "setConfig":{
           "parameters": {
             "volume": {
               "type": "integer",
               "minimum": 0,
               "maximum": 100
             },
             "isMuted": {"type": "boolean"}
           }
        }
      }
    })");
    device->AddCommandHandler("onOff.setConfig",
                              base::Bind(&SpeakerHandler::OnOnOffSetConfig,
                                         weak_ptr_factory_.GetWeakPtr()));
    device->AddCommandHandler("volume.setConfig",
                              base::Bind(&SpeakerHandler::OnVolumeSetConfig,
                                         weak_ptr_factory_.GetWeakPtr()));
  }

 private:
  void OnVolumeSetConfig(const std::weak_ptr<weave::Command>& command) {
    auto cmd = command.lock();
    if (!cmd)
      return;
    LOG(INFO) << "received command: " << cmd->GetName();

    // Handle volume parameter
    int32_t volume_value = 0;
    if (cmd->GetParameters()->GetInteger("volume", &volume_value)) {
      // Display this command in terminal.
      LOG(INFO) << cmd->GetName() << " volume: " << volume_value;

      if (volume_value_ != volume_value) {
        volume_value_ = volume_value;
        UpdateSpeakerState();
      }
      cmd->Complete({}, nullptr);
      return;
    }

    // Handle isMuted parameter
    bool isMuted_status = false;
    if (cmd->GetParameters()->GetBoolean("isMuted", &isMuted_status)) {
      // Display this command in terminal.
      LOG(INFO) << cmd->GetName() << " is "
                << (isMuted_status ? "muted" : "not muted");

      if (isMuted_status_ != isMuted_status) {
        isMuted_status_ = isMuted_status;

        LOG(INFO) << "Speaker is now: "
                  << (isMuted_status ? "muted" : "not muted");
        UpdateSpeakerState();
      }
    }

    cmd->Complete({}, nullptr);
  }

  void OnOnOffSetConfig(const std::weak_ptr<weave::Command>& command) {
    auto cmd = command.lock();
    if (!cmd)
      return;
    LOG(INFO) << "received command: " << cmd->GetName();
    std::string requested_state;
    if (cmd->GetParameters()->GetString("state", &requested_state)) {
      LOG(INFO) << cmd->GetName() << " state: " << requested_state;

      bool new_speaker_status = requested_state == "on";
      if (new_speaker_status != speaker_status_) {
        speaker_status_ = new_speaker_status;

        LOG(INFO) << "Speaker is now: " << (speaker_status_ ? "ON" : "OFF");
        UpdateSpeakerState();
      }
    }
    cmd->Complete({}, nullptr);
  }

  void UpdateSpeakerState() {
    base::DictionaryValue state;
    state.SetString("onOff.state", speaker_status_ ? "on" : "standby");
    state.SetBoolean("volume.isMuted", isMuted_status_);
    state.SetInteger("volume.volume", volume_value_);
    device_->SetStateProperties(state, nullptr);
  }

  weave::Device* device_{nullptr};

  // Simulate the state of the speaker.
  bool speaker_status_;
  bool isMuted_status_;
  int32_t volume_value_;
  base::WeakPtrFactory<SpeakerHandler> weak_ptr_factory_{this};
};

int main(int argc, char** argv) {
  Daemon::Options opts;
  if (!opts.Parse(argc, argv)) {
    Daemon::Options::ShowUsage(argv[0]);
    return 1;
  }
  Daemon daemon{opts};
  SpeakerHandler speaker;
  speaker.Register(daemon.GetDevice());
  daemon.Run();
  return 0;
}