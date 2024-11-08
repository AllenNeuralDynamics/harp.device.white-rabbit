using Bonsai;
using Bonsai.Harp;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reactive.Linq;
using System.Xml.Serialization;

namespace AllenNeuralDynamics.WhiteRabbit
{
    /// <summary>
    /// Generates events and processes commands for the WhiteRabbit device connected
    /// at the specified serial port.
    /// </summary>
    [Combinator(MethodName = nameof(Generate))]
    [WorkflowElementCategory(ElementCategory.Source)]
    [Description("Generates events and processes commands for the WhiteRabbit device.")]
    public partial class Device : Bonsai.Harp.Device, INamedElement
    {
        /// <summary>
        /// Represents the unique identity class of the <see cref="WhiteRabbit"/> device.
        /// This field is constant.
        /// </summary>
        public const int WhoAmI = 1404;

        /// <summary>
        /// Initializes a new instance of the <see cref="Device"/> class.
        /// </summary>
        public Device() : base(WhoAmI) { }

        string INamedElement.Name => nameof(WhiteRabbit);

        /// <summary>
        /// Gets a read-only mapping from address to register type.
        /// </summary>
        public static new IReadOnlyDictionary<int, Type> RegisterMap { get; } = new Dictionary<int, Type>
            (Bonsai.Harp.Device.RegisterMap.ToDictionary(entry => entry.Key, entry => entry.Value))
        {
            { 32, typeof(ConnectedDevices) },
            { 33, typeof(Counter) },
            { 34, typeof(CounterFrequencyHz) },
            { 35, typeof(AuxPortMode) },
            { 36, typeof(AuxPortBaudRate) }
        };

        /// <summary>
        /// Gets the contents of the metadata file describing the <see cref="WhiteRabbit"/>
        /// device registers.
        /// </summary>
        public static readonly string Metadata = GetDeviceMetadata();

        static string GetDeviceMetadata()
        {
            var deviceType = typeof(Device);
            using var metadataStream = deviceType.Assembly.GetManifestResourceStream($"{deviceType.Namespace}.device.yml");
            using var streamReader = new System.IO.StreamReader(metadataStream);
            return streamReader.ReadToEnd();
        }
    }

    /// <summary>
    /// Represents an operator that returns the contents of the metadata file
    /// describing the <see cref="WhiteRabbit"/> device registers.
    /// </summary>
    [Description("Returns the contents of the metadata file describing the WhiteRabbit device registers.")]
    public partial class GetMetadata : Source<string>
    {
        /// <summary>
        /// Returns an observable sequence with the contents of the metadata file
        /// describing the <see cref="WhiteRabbit"/> device registers.
        /// </summary>
        /// <returns>
        /// A sequence with a single <see cref="string"/> object representing the
        /// contents of the metadata file.
        /// </returns>
        public override IObservable<string> Generate()
        {
            return Observable.Return(Device.Metadata);
        }
    }

    /// <summary>
    /// Represents an operator that groups the sequence of <see cref="WhiteRabbit"/>" messages by register type.
    /// </summary>
    [Description("Groups the sequence of WhiteRabbit messages by register type.")]
    public partial class GroupByRegister : Combinator<HarpMessage, IGroupedObservable<Type, HarpMessage>>
    {
        /// <summary>
        /// Groups an observable sequence of <see cref="WhiteRabbit"/> messages
        /// by register type.
        /// </summary>
        /// <param name="source">The sequence of Harp device messages.</param>
        /// <returns>
        /// A sequence of observable groups, each of which corresponds to a unique
        /// <see cref="WhiteRabbit"/> register.
        /// </returns>
        public override IObservable<IGroupedObservable<Type, HarpMessage>> Process(IObservable<HarpMessage> source)
        {
            return source.GroupBy(message => Device.RegisterMap[message.Address]);
        }
    }

    /// <summary>
    /// Represents an operator that filters register-specific messages
    /// reported by the <see cref="WhiteRabbit"/> device.
    /// </summary>
    /// <seealso cref="ConnectedDevices"/>
    /// <seealso cref="Counter"/>
    /// <seealso cref="CounterFrequencyHz"/>
    /// <seealso cref="AuxPortMode"/>
    /// <seealso cref="AuxPortBaudRate"/>
    [XmlInclude(typeof(ConnectedDevices))]
    [XmlInclude(typeof(Counter))]
    [XmlInclude(typeof(CounterFrequencyHz))]
    [XmlInclude(typeof(AuxPortMode))]
    [XmlInclude(typeof(AuxPortBaudRate))]
    [Description("Filters register-specific messages reported by the WhiteRabbit device.")]
    public class FilterRegister : FilterRegisterBuilder, INamedElement
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="FilterRegister"/> class.
        /// </summary>
        public FilterRegister()
        {
            Register = new ConnectedDevices();
        }

        string INamedElement.Name
        {
            get => $"{nameof(WhiteRabbit)}.{GetElementDisplayName(Register)}";
        }
    }

    /// <summary>
    /// Represents an operator which filters and selects specific messages
    /// reported by the WhiteRabbit device.
    /// </summary>
    /// <seealso cref="ConnectedDevices"/>
    /// <seealso cref="Counter"/>
    /// <seealso cref="CounterFrequencyHz"/>
    /// <seealso cref="AuxPortMode"/>
    /// <seealso cref="AuxPortBaudRate"/>
    [XmlInclude(typeof(ConnectedDevices))]
    [XmlInclude(typeof(Counter))]
    [XmlInclude(typeof(CounterFrequencyHz))]
    [XmlInclude(typeof(AuxPortMode))]
    [XmlInclude(typeof(AuxPortBaudRate))]
    [XmlInclude(typeof(TimestampedConnectedDevices))]
    [XmlInclude(typeof(TimestampedCounter))]
    [XmlInclude(typeof(TimestampedCounterFrequencyHz))]
    [XmlInclude(typeof(TimestampedAuxPortMode))]
    [XmlInclude(typeof(TimestampedAuxPortBaudRate))]
    [Description("Filters and selects specific messages reported by the WhiteRabbit device.")]
    public partial class Parse : ParseBuilder, INamedElement
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="Parse"/> class.
        /// </summary>
        public Parse()
        {
            Register = new ConnectedDevices();
        }

        string INamedElement.Name => $"{nameof(WhiteRabbit)}.{GetElementDisplayName(Register)}";
    }

    /// <summary>
    /// Represents an operator which formats a sequence of values as specific
    /// WhiteRabbit register messages.
    /// </summary>
    /// <seealso cref="ConnectedDevices"/>
    /// <seealso cref="Counter"/>
    /// <seealso cref="CounterFrequencyHz"/>
    /// <seealso cref="AuxPortMode"/>
    /// <seealso cref="AuxPortBaudRate"/>
    [XmlInclude(typeof(ConnectedDevices))]
    [XmlInclude(typeof(Counter))]
    [XmlInclude(typeof(CounterFrequencyHz))]
    [XmlInclude(typeof(AuxPortMode))]
    [XmlInclude(typeof(AuxPortBaudRate))]
    [Description("Formats a sequence of values as specific WhiteRabbit register messages.")]
    public partial class Format : FormatBuilder, INamedElement
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="Format"/> class.
        /// </summary>
        public Format()
        {
            Register = new ConnectedDevices();
        }

        string INamedElement.Name => $"{nameof(WhiteRabbit)}.{GetElementDisplayName(Register)}";
    }

    /// <summary>
    /// Represents a register that the currently connected output channels. An event will be generated when any of the channels are connected or disconnected.
    /// </summary>
    [Description("The currently connected output channels. An event will be generated when any of the channels are connected or disconnected.")]
    public partial class ConnectedDevices
    {
        /// <summary>
        /// Represents the address of the <see cref="ConnectedDevices"/> register. This field is constant.
        /// </summary>
        public const int Address = 32;

        /// <summary>
        /// Represents the payload type of the <see cref="ConnectedDevices"/> register. This field is constant.
        /// </summary>
        public const PayloadType RegisterType = PayloadType.U16;

        /// <summary>
        /// Represents the length of the <see cref="ConnectedDevices"/> register. This field is constant.
        /// </summary>
        public const int RegisterLength = 1;

        /// <summary>
        /// Returns the payload data for <see cref="ConnectedDevices"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the message payload.</returns>
        public static ClockOutChannels GetPayload(HarpMessage message)
        {
            return (ClockOutChannels)message.GetPayloadUInt16();
        }

        /// <summary>
        /// Returns the timestamped payload data for <see cref="ConnectedDevices"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<ClockOutChannels> GetTimestampedPayload(HarpMessage message)
        {
            var payload = message.GetTimestampedPayloadUInt16();
            return Timestamped.Create((ClockOutChannels)payload.Value, payload.Seconds);
        }

        /// <summary>
        /// Returns a Harp message for the <see cref="ConnectedDevices"/> register.
        /// </summary>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="ConnectedDevices"/> register
        /// with the specified message type and payload.
        /// </returns>
        public static HarpMessage FromPayload(MessageType messageType, ClockOutChannels value)
        {
            return HarpMessage.FromUInt16(Address, messageType, (ushort)value);
        }

        /// <summary>
        /// Returns a timestamped Harp message for the <see cref="ConnectedDevices"/>
        /// register.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="ConnectedDevices"/> register
        /// with the specified message type, timestamp, and payload.
        /// </returns>
        public static HarpMessage FromPayload(double timestamp, MessageType messageType, ClockOutChannels value)
        {
            return HarpMessage.FromUInt16(Address, timestamp, messageType, (ushort)value);
        }
    }

    /// <summary>
    /// Provides methods for manipulating timestamped messages from the
    /// ConnectedDevices register.
    /// </summary>
    /// <seealso cref="ConnectedDevices"/>
    [Description("Filters and selects timestamped messages from the ConnectedDevices register.")]
    public partial class TimestampedConnectedDevices
    {
        /// <summary>
        /// Represents the address of the <see cref="ConnectedDevices"/> register. This field is constant.
        /// </summary>
        public const int Address = ConnectedDevices.Address;

        /// <summary>
        /// Returns timestamped payload data for <see cref="ConnectedDevices"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<ClockOutChannels> GetPayload(HarpMessage message)
        {
            return ConnectedDevices.GetTimestampedPayload(message);
        }
    }

    /// <summary>
    /// Represents a register that the counter value. This value is incremented at the frequency specified by CounterFrequencyHz. Write to force a counter value.
    /// </summary>
    [Description("The counter value. This value is incremented at the frequency specified by CounterFrequencyHz. Write to force a counter value.")]
    public partial class Counter
    {
        /// <summary>
        /// Represents the address of the <see cref="Counter"/> register. This field is constant.
        /// </summary>
        public const int Address = 33;

        /// <summary>
        /// Represents the payload type of the <see cref="Counter"/> register. This field is constant.
        /// </summary>
        public const PayloadType RegisterType = PayloadType.U32;

        /// <summary>
        /// Represents the length of the <see cref="Counter"/> register. This field is constant.
        /// </summary>
        public const int RegisterLength = 1;

        /// <summary>
        /// Returns the payload data for <see cref="Counter"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the message payload.</returns>
        public static uint GetPayload(HarpMessage message)
        {
            return message.GetPayloadUInt32();
        }

        /// <summary>
        /// Returns the timestamped payload data for <see cref="Counter"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<uint> GetTimestampedPayload(HarpMessage message)
        {
            return message.GetTimestampedPayloadUInt32();
        }

        /// <summary>
        /// Returns a Harp message for the <see cref="Counter"/> register.
        /// </summary>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="Counter"/> register
        /// with the specified message type and payload.
        /// </returns>
        public static HarpMessage FromPayload(MessageType messageType, uint value)
        {
            return HarpMessage.FromUInt32(Address, messageType, value);
        }

        /// <summary>
        /// Returns a timestamped Harp message for the <see cref="Counter"/>
        /// register.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="Counter"/> register
        /// with the specified message type, timestamp, and payload.
        /// </returns>
        public static HarpMessage FromPayload(double timestamp, MessageType messageType, uint value)
        {
            return HarpMessage.FromUInt32(Address, timestamp, messageType, value);
        }
    }

    /// <summary>
    /// Provides methods for manipulating timestamped messages from the
    /// Counter register.
    /// </summary>
    /// <seealso cref="Counter"/>
    [Description("Filters and selects timestamped messages from the Counter register.")]
    public partial class TimestampedCounter
    {
        /// <summary>
        /// Represents the address of the <see cref="Counter"/> register. This field is constant.
        /// </summary>
        public const int Address = Counter.Address;

        /// <summary>
        /// Returns timestamped payload data for <see cref="Counter"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<uint> GetPayload(HarpMessage message)
        {
            return Counter.GetTimestampedPayload(message);
        }
    }

    /// <summary>
    /// Represents a register that the frequency at which the counter is incremented. A value of 0 disables the counter.
    /// </summary>
    [Description("The frequency at which the counter is incremented. A value of 0 disables the counter.")]
    public partial class CounterFrequencyHz
    {
        /// <summary>
        /// Represents the address of the <see cref="CounterFrequencyHz"/> register. This field is constant.
        /// </summary>
        public const int Address = 34;

        /// <summary>
        /// Represents the payload type of the <see cref="CounterFrequencyHz"/> register. This field is constant.
        /// </summary>
        public const PayloadType RegisterType = PayloadType.U16;

        /// <summary>
        /// Represents the length of the <see cref="CounterFrequencyHz"/> register. This field is constant.
        /// </summary>
        public const int RegisterLength = 1;

        /// <summary>
        /// Returns the payload data for <see cref="CounterFrequencyHz"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the message payload.</returns>
        public static ushort GetPayload(HarpMessage message)
        {
            return message.GetPayloadUInt16();
        }

        /// <summary>
        /// Returns the timestamped payload data for <see cref="CounterFrequencyHz"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<ushort> GetTimestampedPayload(HarpMessage message)
        {
            return message.GetTimestampedPayloadUInt16();
        }

        /// <summary>
        /// Returns a Harp message for the <see cref="CounterFrequencyHz"/> register.
        /// </summary>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="CounterFrequencyHz"/> register
        /// with the specified message type and payload.
        /// </returns>
        public static HarpMessage FromPayload(MessageType messageType, ushort value)
        {
            return HarpMessage.FromUInt16(Address, messageType, value);
        }

        /// <summary>
        /// Returns a timestamped Harp message for the <see cref="CounterFrequencyHz"/>
        /// register.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="CounterFrequencyHz"/> register
        /// with the specified message type, timestamp, and payload.
        /// </returns>
        public static HarpMessage FromPayload(double timestamp, MessageType messageType, ushort value)
        {
            return HarpMessage.FromUInt16(Address, timestamp, messageType, value);
        }
    }

    /// <summary>
    /// Provides methods for manipulating timestamped messages from the
    /// CounterFrequencyHz register.
    /// </summary>
    /// <seealso cref="CounterFrequencyHz"/>
    [Description("Filters and selects timestamped messages from the CounterFrequencyHz register.")]
    public partial class TimestampedCounterFrequencyHz
    {
        /// <summary>
        /// Represents the address of the <see cref="CounterFrequencyHz"/> register. This field is constant.
        /// </summary>
        public const int Address = CounterFrequencyHz.Address;

        /// <summary>
        /// Returns timestamped payload data for <see cref="CounterFrequencyHz"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<ushort> GetPayload(HarpMessage message)
        {
            return CounterFrequencyHz.GetTimestampedPayload(message);
        }
    }

    /// <summary>
    /// Represents a register that the function of the auxiliary port.
    /// </summary>
    [Description("The function of the auxiliary port.")]
    public partial class AuxPortMode
    {
        /// <summary>
        /// Represents the address of the <see cref="AuxPortMode"/> register. This field is constant.
        /// </summary>
        public const int Address = 35;

        /// <summary>
        /// Represents the payload type of the <see cref="AuxPortMode"/> register. This field is constant.
        /// </summary>
        public const PayloadType RegisterType = PayloadType.U8;

        /// <summary>
        /// Represents the length of the <see cref="AuxPortMode"/> register. This field is constant.
        /// </summary>
        public const int RegisterLength = 1;

        /// <summary>
        /// Returns the payload data for <see cref="AuxPortMode"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the message payload.</returns>
        public static AuxPortModeConfig GetPayload(HarpMessage message)
        {
            return (AuxPortModeConfig)message.GetPayloadByte();
        }

        /// <summary>
        /// Returns the timestamped payload data for <see cref="AuxPortMode"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<AuxPortModeConfig> GetTimestampedPayload(HarpMessage message)
        {
            var payload = message.GetTimestampedPayloadByte();
            return Timestamped.Create((AuxPortModeConfig)payload.Value, payload.Seconds);
        }

        /// <summary>
        /// Returns a Harp message for the <see cref="AuxPortMode"/> register.
        /// </summary>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="AuxPortMode"/> register
        /// with the specified message type and payload.
        /// </returns>
        public static HarpMessage FromPayload(MessageType messageType, AuxPortModeConfig value)
        {
            return HarpMessage.FromByte(Address, messageType, (byte)value);
        }

        /// <summary>
        /// Returns a timestamped Harp message for the <see cref="AuxPortMode"/>
        /// register.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="AuxPortMode"/> register
        /// with the specified message type, timestamp, and payload.
        /// </returns>
        public static HarpMessage FromPayload(double timestamp, MessageType messageType, AuxPortModeConfig value)
        {
            return HarpMessage.FromByte(Address, timestamp, messageType, (byte)value);
        }
    }

    /// <summary>
    /// Provides methods for manipulating timestamped messages from the
    /// AuxPortMode register.
    /// </summary>
    /// <seealso cref="AuxPortMode"/>
    [Description("Filters and selects timestamped messages from the AuxPortMode register.")]
    public partial class TimestampedAuxPortMode
    {
        /// <summary>
        /// Represents the address of the <see cref="AuxPortMode"/> register. This field is constant.
        /// </summary>
        public const int Address = AuxPortMode.Address;

        /// <summary>
        /// Returns timestamped payload data for <see cref="AuxPortMode"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<AuxPortModeConfig> GetPayload(HarpMessage message)
        {
            return AuxPortMode.GetTimestampedPayload(message);
        }
    }

    /// <summary>
    /// Represents a register that the baud rate, in bps, of the auxiliary port when in HarpClock mode.
    /// </summary>
    [Description("The baud rate, in bps, of the auxiliary port when in HarpClock mode.")]
    public partial class AuxPortBaudRate
    {
        /// <summary>
        /// Represents the address of the <see cref="AuxPortBaudRate"/> register. This field is constant.
        /// </summary>
        public const int Address = 36;

        /// <summary>
        /// Represents the payload type of the <see cref="AuxPortBaudRate"/> register. This field is constant.
        /// </summary>
        public const PayloadType RegisterType = PayloadType.U32;

        /// <summary>
        /// Represents the length of the <see cref="AuxPortBaudRate"/> register. This field is constant.
        /// </summary>
        public const int RegisterLength = 1;

        /// <summary>
        /// Returns the payload data for <see cref="AuxPortBaudRate"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the message payload.</returns>
        public static uint GetPayload(HarpMessage message)
        {
            return message.GetPayloadUInt32();
        }

        /// <summary>
        /// Returns the timestamped payload data for <see cref="AuxPortBaudRate"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<uint> GetTimestampedPayload(HarpMessage message)
        {
            return message.GetTimestampedPayloadUInt32();
        }

        /// <summary>
        /// Returns a Harp message for the <see cref="AuxPortBaudRate"/> register.
        /// </summary>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="AuxPortBaudRate"/> register
        /// with the specified message type and payload.
        /// </returns>
        public static HarpMessage FromPayload(MessageType messageType, uint value)
        {
            return HarpMessage.FromUInt32(Address, messageType, value);
        }

        /// <summary>
        /// Returns a timestamped Harp message for the <see cref="AuxPortBaudRate"/>
        /// register.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">The type of the Harp message.</param>
        /// <param name="value">The value to be stored in the message payload.</param>
        /// <returns>
        /// A <see cref="HarpMessage"/> object for the <see cref="AuxPortBaudRate"/> register
        /// with the specified message type, timestamp, and payload.
        /// </returns>
        public static HarpMessage FromPayload(double timestamp, MessageType messageType, uint value)
        {
            return HarpMessage.FromUInt32(Address, timestamp, messageType, value);
        }
    }

    /// <summary>
    /// Provides methods for manipulating timestamped messages from the
    /// AuxPortBaudRate register.
    /// </summary>
    /// <seealso cref="AuxPortBaudRate"/>
    [Description("Filters and selects timestamped messages from the AuxPortBaudRate register.")]
    public partial class TimestampedAuxPortBaudRate
    {
        /// <summary>
        /// Represents the address of the <see cref="AuxPortBaudRate"/> register. This field is constant.
        /// </summary>
        public const int Address = AuxPortBaudRate.Address;

        /// <summary>
        /// Returns timestamped payload data for <see cref="AuxPortBaudRate"/> register messages.
        /// </summary>
        /// <param name="message">A <see cref="HarpMessage"/> object representing the register message.</param>
        /// <returns>A value representing the timestamped message payload.</returns>
        public static Timestamped<uint> GetPayload(HarpMessage message)
        {
            return AuxPortBaudRate.GetTimestampedPayload(message);
        }
    }

    /// <summary>
    /// Represents an operator which creates standard message payloads for the
    /// WhiteRabbit device.
    /// </summary>
    /// <seealso cref="CreateConnectedDevicesPayload"/>
    /// <seealso cref="CreateCounterPayload"/>
    /// <seealso cref="CreateCounterFrequencyHzPayload"/>
    /// <seealso cref="CreateAuxPortModePayload"/>
    /// <seealso cref="CreateAuxPortBaudRatePayload"/>
    [XmlInclude(typeof(CreateConnectedDevicesPayload))]
    [XmlInclude(typeof(CreateCounterPayload))]
    [XmlInclude(typeof(CreateCounterFrequencyHzPayload))]
    [XmlInclude(typeof(CreateAuxPortModePayload))]
    [XmlInclude(typeof(CreateAuxPortBaudRatePayload))]
    [XmlInclude(typeof(CreateTimestampedConnectedDevicesPayload))]
    [XmlInclude(typeof(CreateTimestampedCounterPayload))]
    [XmlInclude(typeof(CreateTimestampedCounterFrequencyHzPayload))]
    [XmlInclude(typeof(CreateTimestampedAuxPortModePayload))]
    [XmlInclude(typeof(CreateTimestampedAuxPortBaudRatePayload))]
    [Description("Creates standard message payloads for the WhiteRabbit device.")]
    public partial class CreateMessage : CreateMessageBuilder, INamedElement
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="CreateMessage"/> class.
        /// </summary>
        public CreateMessage()
        {
            Payload = new CreateConnectedDevicesPayload();
        }

        string INamedElement.Name => $"{nameof(WhiteRabbit)}.{GetElementDisplayName(Payload)}";
    }

    /// <summary>
    /// Represents an operator that creates a message payload
    /// that the currently connected output channels. An event will be generated when any of the channels are connected or disconnected.
    /// </summary>
    [DisplayName("ConnectedDevicesPayload")]
    [Description("Creates a message payload that the currently connected output channels. An event will be generated when any of the channels are connected or disconnected.")]
    public partial class CreateConnectedDevicesPayload
    {
        /// <summary>
        /// Gets or sets the value that the currently connected output channels. An event will be generated when any of the channels are connected or disconnected.
        /// </summary>
        [Description("The value that the currently connected output channels. An event will be generated when any of the channels are connected or disconnected.")]
        public ClockOutChannels ConnectedDevices { get; set; }

        /// <summary>
        /// Creates a message payload for the ConnectedDevices register.
        /// </summary>
        /// <returns>The created message payload value.</returns>
        public ClockOutChannels GetPayload()
        {
            return ConnectedDevices;
        }

        /// <summary>
        /// Creates a message that the currently connected output channels. An event will be generated when any of the channels are connected or disconnected.
        /// </summary>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new message for the ConnectedDevices register.</returns>
        public HarpMessage GetMessage(MessageType messageType)
        {
            return AllenNeuralDynamics.WhiteRabbit.ConnectedDevices.FromPayload(messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a timestamped message payload
    /// that the currently connected output channels. An event will be generated when any of the channels are connected or disconnected.
    /// </summary>
    [DisplayName("TimestampedConnectedDevicesPayload")]
    [Description("Creates a timestamped message payload that the currently connected output channels. An event will be generated when any of the channels are connected or disconnected.")]
    public partial class CreateTimestampedConnectedDevicesPayload : CreateConnectedDevicesPayload
    {
        /// <summary>
        /// Creates a timestamped message that the currently connected output channels. An event will be generated when any of the channels are connected or disconnected.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new timestamped message for the ConnectedDevices register.</returns>
        public HarpMessage GetMessage(double timestamp, MessageType messageType)
        {
            return AllenNeuralDynamics.WhiteRabbit.ConnectedDevices.FromPayload(timestamp, messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a message payload
    /// that the counter value. This value is incremented at the frequency specified by CounterFrequencyHz. Write to force a counter value.
    /// </summary>
    [DisplayName("CounterPayload")]
    [Description("Creates a message payload that the counter value. This value is incremented at the frequency specified by CounterFrequencyHz. Write to force a counter value.")]
    public partial class CreateCounterPayload
    {
        /// <summary>
        /// Gets or sets the value that the counter value. This value is incremented at the frequency specified by CounterFrequencyHz. Write to force a counter value.
        /// </summary>
        [Description("The value that the counter value. This value is incremented at the frequency specified by CounterFrequencyHz. Write to force a counter value.")]
        public uint Counter { get; set; }

        /// <summary>
        /// Creates a message payload for the Counter register.
        /// </summary>
        /// <returns>The created message payload value.</returns>
        public uint GetPayload()
        {
            return Counter;
        }

        /// <summary>
        /// Creates a message that the counter value. This value is incremented at the frequency specified by CounterFrequencyHz. Write to force a counter value.
        /// </summary>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new message for the Counter register.</returns>
        public HarpMessage GetMessage(MessageType messageType)
        {
            return AllenNeuralDynamics.WhiteRabbit.Counter.FromPayload(messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a timestamped message payload
    /// that the counter value. This value is incremented at the frequency specified by CounterFrequencyHz. Write to force a counter value.
    /// </summary>
    [DisplayName("TimestampedCounterPayload")]
    [Description("Creates a timestamped message payload that the counter value. This value is incremented at the frequency specified by CounterFrequencyHz. Write to force a counter value.")]
    public partial class CreateTimestampedCounterPayload : CreateCounterPayload
    {
        /// <summary>
        /// Creates a timestamped message that the counter value. This value is incremented at the frequency specified by CounterFrequencyHz. Write to force a counter value.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new timestamped message for the Counter register.</returns>
        public HarpMessage GetMessage(double timestamp, MessageType messageType)
        {
            return AllenNeuralDynamics.WhiteRabbit.Counter.FromPayload(timestamp, messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a message payload
    /// that the frequency at which the counter is incremented. A value of 0 disables the counter.
    /// </summary>
    [DisplayName("CounterFrequencyHzPayload")]
    [Description("Creates a message payload that the frequency at which the counter is incremented. A value of 0 disables the counter.")]
    public partial class CreateCounterFrequencyHzPayload
    {
        /// <summary>
        /// Gets or sets the value that the frequency at which the counter is incremented. A value of 0 disables the counter.
        /// </summary>
        [Range(min: 0, max: 500)]
        [Editor(DesignTypes.NumericUpDownEditor, DesignTypes.UITypeEditor)]
        [Description("The value that the frequency at which the counter is incremented. A value of 0 disables the counter.")]
        public ushort CounterFrequencyHz { get; set; } = 0;

        /// <summary>
        /// Creates a message payload for the CounterFrequencyHz register.
        /// </summary>
        /// <returns>The created message payload value.</returns>
        public ushort GetPayload()
        {
            return CounterFrequencyHz;
        }

        /// <summary>
        /// Creates a message that the frequency at which the counter is incremented. A value of 0 disables the counter.
        /// </summary>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new message for the CounterFrequencyHz register.</returns>
        public HarpMessage GetMessage(MessageType messageType)
        {
            return AllenNeuralDynamics.WhiteRabbit.CounterFrequencyHz.FromPayload(messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a timestamped message payload
    /// that the frequency at which the counter is incremented. A value of 0 disables the counter.
    /// </summary>
    [DisplayName("TimestampedCounterFrequencyHzPayload")]
    [Description("Creates a timestamped message payload that the frequency at which the counter is incremented. A value of 0 disables the counter.")]
    public partial class CreateTimestampedCounterFrequencyHzPayload : CreateCounterFrequencyHzPayload
    {
        /// <summary>
        /// Creates a timestamped message that the frequency at which the counter is incremented. A value of 0 disables the counter.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new timestamped message for the CounterFrequencyHz register.</returns>
        public HarpMessage GetMessage(double timestamp, MessageType messageType)
        {
            return AllenNeuralDynamics.WhiteRabbit.CounterFrequencyHz.FromPayload(timestamp, messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a message payload
    /// that the function of the auxiliary port.
    /// </summary>
    [DisplayName("AuxPortModePayload")]
    [Description("Creates a message payload that the function of the auxiliary port.")]
    public partial class CreateAuxPortModePayload
    {
        /// <summary>
        /// Gets or sets the value that the function of the auxiliary port.
        /// </summary>
        [Description("The value that the function of the auxiliary port.")]
        public AuxPortModeConfig AuxPortMode { get; set; }

        /// <summary>
        /// Creates a message payload for the AuxPortMode register.
        /// </summary>
        /// <returns>The created message payload value.</returns>
        public AuxPortModeConfig GetPayload()
        {
            return AuxPortMode;
        }

        /// <summary>
        /// Creates a message that the function of the auxiliary port.
        /// </summary>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new message for the AuxPortMode register.</returns>
        public HarpMessage GetMessage(MessageType messageType)
        {
            return AllenNeuralDynamics.WhiteRabbit.AuxPortMode.FromPayload(messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a timestamped message payload
    /// that the function of the auxiliary port.
    /// </summary>
    [DisplayName("TimestampedAuxPortModePayload")]
    [Description("Creates a timestamped message payload that the function of the auxiliary port.")]
    public partial class CreateTimestampedAuxPortModePayload : CreateAuxPortModePayload
    {
        /// <summary>
        /// Creates a timestamped message that the function of the auxiliary port.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new timestamped message for the AuxPortMode register.</returns>
        public HarpMessage GetMessage(double timestamp, MessageType messageType)
        {
            return AllenNeuralDynamics.WhiteRabbit.AuxPortMode.FromPayload(timestamp, messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a message payload
    /// that the baud rate, in bps, of the auxiliary port when in HarpClock mode.
    /// </summary>
    [DisplayName("AuxPortBaudRatePayload")]
    [Description("Creates a message payload that the baud rate, in bps, of the auxiliary port when in HarpClock mode.")]
    public partial class CreateAuxPortBaudRatePayload
    {
        /// <summary>
        /// Gets or sets the value that the baud rate, in bps, of the auxiliary port when in HarpClock mode.
        /// </summary>
        [Range(min: 40, max: 1000000)]
        [Editor(DesignTypes.NumericUpDownEditor, DesignTypes.UITypeEditor)]
        [Description("The value that the baud rate, in bps, of the auxiliary port when in HarpClock mode.")]
        public uint AuxPortBaudRate { get; set; } = 1000;

        /// <summary>
        /// Creates a message payload for the AuxPortBaudRate register.
        /// </summary>
        /// <returns>The created message payload value.</returns>
        public uint GetPayload()
        {
            return AuxPortBaudRate;
        }

        /// <summary>
        /// Creates a message that the baud rate, in bps, of the auxiliary port when in HarpClock mode.
        /// </summary>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new message for the AuxPortBaudRate register.</returns>
        public HarpMessage GetMessage(MessageType messageType)
        {
            return AllenNeuralDynamics.WhiteRabbit.AuxPortBaudRate.FromPayload(messageType, GetPayload());
        }
    }

    /// <summary>
    /// Represents an operator that creates a timestamped message payload
    /// that the baud rate, in bps, of the auxiliary port when in HarpClock mode.
    /// </summary>
    [DisplayName("TimestampedAuxPortBaudRatePayload")]
    [Description("Creates a timestamped message payload that the baud rate, in bps, of the auxiliary port when in HarpClock mode.")]
    public partial class CreateTimestampedAuxPortBaudRatePayload : CreateAuxPortBaudRatePayload
    {
        /// <summary>
        /// Creates a timestamped message that the baud rate, in bps, of the auxiliary port when in HarpClock mode.
        /// </summary>
        /// <param name="timestamp">The timestamp of the message payload, in seconds.</param>
        /// <param name="messageType">Specifies the type of the created message.</param>
        /// <returns>A new timestamped message for the AuxPortBaudRate register.</returns>
        public HarpMessage GetMessage(double timestamp, MessageType messageType)
        {
            return AllenNeuralDynamics.WhiteRabbit.AuxPortBaudRate.FromPayload(timestamp, messageType, GetPayload());
        }
    }

    /// <summary>
    /// Clock output channels
    /// </summary>
    [Flags]
    public enum ClockOutChannels : ushort
    {
        None = 0x0,
        Channel0 = 0x1,
        Channel1 = 0x2,
        Channel2 = 0x4,
        Channel3 = 0x8,
        Channel4 = 0x10,
        Channel5 = 0x20,
        Channel6 = 0x40,
        Channel7 = 0x80,
        Channel8 = 0x100,
        Channel9 = 0x200,
        Channel10 = 0x400,
        Channel11 = 0x800,
        Channel12 = 0x1000,
        Channel13 = 0x2000,
        Channel14 = 0x4000,
        Channel15 = 0x8000
    }

    /// <summary>
    /// Auxiliary port available configuration
    /// </summary>
    public enum AuxPortModeConfig : byte
    {
        Disabled = 0,
        HarpClock = 1,
        PPS = 2
    }
}
