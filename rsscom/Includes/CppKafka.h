/*
 * Copyright (c) 2017, Matias Fontanini
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following disclaimer
 *   in the documentation and/or other materials provided with the
 *   distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef CPPKAFKA_H
#define CPPKAFKA_H

#include <Buffer.h>
#include <ClonablePtr.h>
#include <Configuration.h>
#include <ConfigurationBase.h>
#include <ConfigurationOption.h>
#include <Consumer.h>
#include <Error.h>
#include <Event.h>
#include <Exceptions.h>
#include <GroupInformation.h>
#include <Header.h>
#include <HeaderList.h>
#include <HeaderListIterator.h>
#include <KafkaHandleBase.h>
#include <Logging.h>
#include <Macros.h>
#include <Message.h>
#include <MessageBuilder.h>
#include <MessageInternal.h>
#include <MessageTimestamp.h>
#include <Metadata.h>
#include <Producer.h>
#include <Queue.h>
#include <Topic.h>
#include <TopicConfiguration.h>
#include <TopicPartition.h>
#include <TopicPartitionList.h>
#include <utils/BackoffCommitter.h>
#include <utils/BackoffPerformer.h>
#include <utils/BufferedProducer.h>
#include <utils/CompactedTopicProcessor.h>
#include <utils/ConsumerDispatcher.h>
#include <utils/PollInterface.h>
#include <utils/PollStrategyBase.h>
#include <utils/RoundrobinPollStrategy.h>

#endif
