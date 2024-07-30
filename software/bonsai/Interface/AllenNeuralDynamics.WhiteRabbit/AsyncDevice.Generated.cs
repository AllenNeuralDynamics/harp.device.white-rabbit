using Bonsai.Harp;
using System.Threading;
using System.Threading.Tasks;

namespace AllenNeuralDynamics.WhiteRabbit
{
    /// <inheritdoc/>
    public partial class Device
    {
        /// <summary>
        /// Initializes a new instance of the asynchronous API to configure and interface
        /// with WhiteRabbit devices on the specified serial port.
        /// </summary>
        /// <param name="portName">
        /// The name of the serial port used to communicate with the Harp device.
        /// </param>
        /// <returns>
        /// A task that represents the asynchronous initialization operation. The value of
        /// the <see cref="Task{TResult}.Result"/> parameter contains a new instance of
        /// the <see cref="AsyncDevice"/> class.
        /// </returns>
        public static async Task<AsyncDevice> CreateAsync(string portName)
        {
            var device = new AsyncDevice(portName);
            var whoAmI = await device.ReadWhoAmIAsync();
            if (whoAmI != Device.WhoAmI)
            {
                var errorMessage = string.Format(
                    "The device ID {1} on {0} was unexpected. Check whether a WhiteRabbit device is connected to the specified serial port.",
                    portName, whoAmI);
                throw new HarpException(errorMessage);
            }

            return device;
        }
    }

    /// <summary>
    /// Represents an asynchronous API to configure and interface with WhiteRabbit devices.
    /// </summary>
    public partial class AsyncDevice : Bonsai.Harp.AsyncDevice
    {
        internal AsyncDevice(string portName)
            : base(portName)
        {
        }

        /// <summary>
        /// Asynchronously reads the contents of the ConnectedDevices register.
        /// </summary>
        /// <param name="cancellationToken">
        /// A <see cref="CancellationToken"/> which can be used to cancel the operation.
        /// </param>
        /// <returns>
        /// A task that represents the asynchronous read operation. The <see cref="Task{TResult}.Result"/>
        /// property contains the register payload.
        /// </returns>
        public async Task<ClockOutChannels> ReadConnectedDevicesAsync(CancellationToken cancellationToken = default)
        {
            var reply = await CommandAsync(HarpCommand.ReadUInt16(ConnectedDevices.Address), cancellationToken);
            return ConnectedDevices.GetPayload(reply);
        }

        /// <summary>
        /// Asynchronously reads the timestamped contents of the ConnectedDevices register.
        /// </summary>
        /// <param name="cancellationToken">
        /// A <see cref="CancellationToken"/> which can be used to cancel the operation.
        /// </param>
        /// <returns>
        /// A task that represents the asynchronous read operation. The <see cref="Task{TResult}.Result"/>
        /// property contains the timestamped register payload.
        /// </returns>
        public async Task<Timestamped<ClockOutChannels>> ReadTimestampedConnectedDevicesAsync(CancellationToken cancellationToken = default)
        {
            var reply = await CommandAsync(HarpCommand.ReadUInt16(ConnectedDevices.Address), cancellationToken);
            return ConnectedDevices.GetTimestampedPayload(reply);
        }

        /// <summary>
        /// Asynchronously reads the contents of the Counter register.
        /// </summary>
        /// <param name="cancellationToken">
        /// A <see cref="CancellationToken"/> which can be used to cancel the operation.
        /// </param>
        /// <returns>
        /// A task that represents the asynchronous read operation. The <see cref="Task{TResult}.Result"/>
        /// property contains the register payload.
        /// </returns>
        public async Task<uint> ReadCounterAsync(CancellationToken cancellationToken = default)
        {
            var reply = await CommandAsync(HarpCommand.ReadUInt32(Counter.Address), cancellationToken);
            return Counter.GetPayload(reply);
        }

        /// <summary>
        /// Asynchronously reads the timestamped contents of the Counter register.
        /// </summary>
        /// <param name="cancellationToken">
        /// A <see cref="CancellationToken"/> which can be used to cancel the operation.
        /// </param>
        /// <returns>
        /// A task that represents the asynchronous read operation. The <see cref="Task{TResult}.Result"/>
        /// property contains the timestamped register payload.
        /// </returns>
        public async Task<Timestamped<uint>> ReadTimestampedCounterAsync(CancellationToken cancellationToken = default)
        {
            var reply = await CommandAsync(HarpCommand.ReadUInt32(Counter.Address), cancellationToken);
            return Counter.GetTimestampedPayload(reply);
        }

        /// <summary>
        /// Asynchronously writes a value to the Counter register.
        /// </summary>
        /// <param name="value">The value to be stored in the register.</param>
        /// <param name="cancellationToken">
        /// A <see cref="CancellationToken"/> which can be used to cancel the operation.
        /// </param>
        /// <returns>The task object representing the asynchronous write operation.</returns>
        public async Task WriteCounterAsync(uint value, CancellationToken cancellationToken = default)
        {
            var request = Counter.FromPayload(MessageType.Write, value);
            await CommandAsync(request, cancellationToken);
        }

        /// <summary>
        /// Asynchronously reads the contents of the CounterFrequencyHz register.
        /// </summary>
        /// <param name="cancellationToken">
        /// A <see cref="CancellationToken"/> which can be used to cancel the operation.
        /// </param>
        /// <returns>
        /// A task that represents the asynchronous read operation. The <see cref="Task{TResult}.Result"/>
        /// property contains the register payload.
        /// </returns>
        public async Task<ushort> ReadCounterFrequencyHzAsync(CancellationToken cancellationToken = default)
        {
            var reply = await CommandAsync(HarpCommand.ReadUInt16(CounterFrequencyHz.Address), cancellationToken);
            return CounterFrequencyHz.GetPayload(reply);
        }

        /// <summary>
        /// Asynchronously reads the timestamped contents of the CounterFrequencyHz register.
        /// </summary>
        /// <param name="cancellationToken">
        /// A <see cref="CancellationToken"/> which can be used to cancel the operation.
        /// </param>
        /// <returns>
        /// A task that represents the asynchronous read operation. The <see cref="Task{TResult}.Result"/>
        /// property contains the timestamped register payload.
        /// </returns>
        public async Task<Timestamped<ushort>> ReadTimestampedCounterFrequencyHzAsync(CancellationToken cancellationToken = default)
        {
            var reply = await CommandAsync(HarpCommand.ReadUInt16(CounterFrequencyHz.Address), cancellationToken);
            return CounterFrequencyHz.GetTimestampedPayload(reply);
        }

        /// <summary>
        /// Asynchronously writes a value to the CounterFrequencyHz register.
        /// </summary>
        /// <param name="value">The value to be stored in the register.</param>
        /// <param name="cancellationToken">
        /// A <see cref="CancellationToken"/> which can be used to cancel the operation.
        /// </param>
        /// <returns>The task object representing the asynchronous write operation.</returns>
        public async Task WriteCounterFrequencyHzAsync(ushort value, CancellationToken cancellationToken = default)
        {
            var request = CounterFrequencyHz.FromPayload(MessageType.Write, value);
            await CommandAsync(request, cancellationToken);
        }

        /// <summary>
        /// Asynchronously reads the contents of the AuxPortMode register.
        /// </summary>
        /// <param name="cancellationToken">
        /// A <see cref="CancellationToken"/> which can be used to cancel the operation.
        /// </param>
        /// <returns>
        /// A task that represents the asynchronous read operation. The <see cref="Task{TResult}.Result"/>
        /// property contains the register payload.
        /// </returns>
        public async Task<AuxPortModeConfig> ReadAuxPortModeAsync(CancellationToken cancellationToken = default)
        {
            var reply = await CommandAsync(HarpCommand.ReadByte(AuxPortMode.Address), cancellationToken);
            return AuxPortMode.GetPayload(reply);
        }

        /// <summary>
        /// Asynchronously reads the timestamped contents of the AuxPortMode register.
        /// </summary>
        /// <param name="cancellationToken">
        /// A <see cref="CancellationToken"/> which can be used to cancel the operation.
        /// </param>
        /// <returns>
        /// A task that represents the asynchronous read operation. The <see cref="Task{TResult}.Result"/>
        /// property contains the timestamped register payload.
        /// </returns>
        public async Task<Timestamped<AuxPortModeConfig>> ReadTimestampedAuxPortModeAsync(CancellationToken cancellationToken = default)
        {
            var reply = await CommandAsync(HarpCommand.ReadByte(AuxPortMode.Address), cancellationToken);
            return AuxPortMode.GetTimestampedPayload(reply);
        }

        /// <summary>
        /// Asynchronously writes a value to the AuxPortMode register.
        /// </summary>
        /// <param name="value">The value to be stored in the register.</param>
        /// <param name="cancellationToken">
        /// A <see cref="CancellationToken"/> which can be used to cancel the operation.
        /// </param>
        /// <returns>The task object representing the asynchronous write operation.</returns>
        public async Task WriteAuxPortModeAsync(AuxPortModeConfig value, CancellationToken cancellationToken = default)
        {
            var request = AuxPortMode.FromPayload(MessageType.Write, value);
            await CommandAsync(request, cancellationToken);
        }
    }
}
