// Copyright 2015 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIBWEAVE_INCLUDE_WEAVE_NETWORK_PROVIDER_H_
#define LIBWEAVE_INCLUDE_WEAVE_NETWORK_PROVIDER_H_

#include <string>

#include <base/callback.h>
#include <weave/error.h>
#include <weave/stream.h>

namespace weave {

enum class NetworkState {
  kOffline = 0,
  kFailure,
  kConnecting,
  kConnected,
};

// Interface with methods to detect network connectivity and opening network
// connections.
class NetworkProvider {
 public:
  // Callback type for AddConnectionChangedCallback.
  using ConnectionChangedCallback = base::Closure;

  // Callback type for OpenSslSocket.
  using OpenSslSocketSuccessCallback =
      base::Callback<void(std::unique_ptr<Stream> stream)>;

  // Subscribes to notification about changes in network connectivity. Changes
  // may include but not limited: interface up or down, new IP was assigned,
  // cable is disconnected.
  virtual void AddConnectionChangedCallback(
      const ConnectionChangedCallback& callback) = 0;

  // Returns current Internet connectivity state
  virtual NetworkState GetConnectionState() const = 0;

  // Opens bidirectional sockets and returns attached stream.
  virtual void OpenSslSocket(
      const std::string& host,
      uint16_t port,
      const OpenSslSocketSuccessCallback& success_callback,
      const ErrorCallback& error_callback) = 0;

 protected:
  virtual ~NetworkProvider() = default;
};

}  // namespace weave

#endif  // LIBWEAVE_INCLUDE_WEAVE_NETWORK_PROVIDER_H_