// Copyright 2020 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <map>
#include <vector>
#include "common/common_types.h"
#include "common/swap.h"
#include "core/hle/service/nvdrv/devices/nvdevice.h"

namespace Service::Nvidia::Devices {
class nvmap;

class nvhost_nvdec_common : public nvdevice {
public:
    explicit nvhost_nvdec_common(Core::System& system, std::shared_ptr<nvmap> nvmap_dev);
    ~nvhost_nvdec_common() override;

    /**
     * Handles an ioctl1 request.
     * @param command The ioctl command id.
     * @param input A buffer containing the input data for the ioctl.
     * @param output A buffer where the output data will be written to.
     * @returns The result code of the ioctl.
     */
    virtual NvResult Ioctl1(Ioctl command, const std::vector<u8>& input, std::vector<u8>& output,
                            IoctlCtrl& ctrl) = 0;

    /**
     * Handles an ioctl2 request.
     * @param command The ioctl command id.
     * @param input A buffer containing the input data for the ioctl.
     * @param inline_input A buffer containing the input data for the ioctl which has been inlined.
     * @param output A buffer where the output data will be written to.
     * @returns The result code of the ioctl.
     */
    virtual NvResult Ioctl2(Ioctl command, const std::vector<u8>& input,
                            const std::vector<u8>& inline_input, std::vector<u8>& output,
                            IoctlCtrl& ctrl) = 0;

    /**
     * Handles an ioctl3 request.
     * @param command The ioctl command id.
     * @param input A buffer containing the input data for the ioctl.
     * @param output A buffer where the output data will be written to.
     * @param inline_output A buffer where the inlined output data will be written to.
     * @returns The result code of the ioctl.
     */
    virtual NvResult Ioctl3(Ioctl command, const std::vector<u8>& input, std::vector<u8>& output,
                            std::vector<u8>& inline_output, IoctlCtrl& ctrl) = 0;

protected:
    class BufferMap final {
    public:
        constexpr BufferMap() = default;

        constexpr BufferMap(GPUVAddr start_addr, std::size_t size)
            : start_addr{start_addr}, end_addr{start_addr + size} {}

        constexpr BufferMap(GPUVAddr start_addr, std::size_t size, VAddr cpu_addr,
                            bool is_allocated)
            : start_addr{start_addr}, end_addr{start_addr + size}, cpu_addr{cpu_addr},
              is_allocated{is_allocated} {}

        constexpr VAddr StartAddr() const {
            return start_addr;
        }

        constexpr VAddr EndAddr() const {
            return end_addr;
        }

        constexpr std::size_t Size() const {
            return end_addr - start_addr;
        }

        constexpr VAddr CpuAddr() const {
            return cpu_addr;
        }

        constexpr bool IsAllocated() const {
            return is_allocated;
        }

    private:
        GPUVAddr start_addr{};
        GPUVAddr end_addr{};
        VAddr cpu_addr{};
        bool is_allocated{};
    };

    struct IoctlSetNvmapFD {
        s32_le nvmap_fd{};
    };
    static_assert(sizeof(IoctlSetNvmapFD) == 4, "IoctlSetNvmapFD is incorrect size");

    struct IoctlSubmitCommandBuffer {
        u32_le id{};
        u32_le offset{};
        u32_le count{};
    };
    static_assert(sizeof(IoctlSubmitCommandBuffer) == 0xC,
                  "IoctlSubmitCommandBuffer is incorrect size");
    struct IoctlSubmit {
        u32_le cmd_buffer_count{};
        u32_le relocation_count{};
        u32_le syncpoint_count{};
        u32_le fence_count{};
    };
    static_assert(sizeof(IoctlSubmit) == 0x10, "IoctlSubmit has incorrect size");

    struct CommandBuffer {
        s32 memory_id{};
        u32 offset{};
        s32 word_count{};
    };
    static_assert(sizeof(CommandBuffer) == 0xC, "CommandBuffer has incorrect size");

    struct Reloc {
        s32 cmdbuffer_memory{};
        s32 cmdbuffer_offset{};
        s32 target{};
        s32 target_offset{};
    };
    static_assert(sizeof(Reloc) == 0x10, "CommandBuffer has incorrect size");

    struct SyncptIncr {
        u32 id{};
        u32 increments{};
    };
    static_assert(sizeof(SyncptIncr) == 0x8, "CommandBuffer has incorrect size");

    struct Fence {
        u32 id{};
        u32 value{};
    };
    static_assert(sizeof(Fence) == 0x8, "CommandBuffer has incorrect size");

    struct IoctlGetSyncpoint {
        // Input
        u32_le param{};
        // Output
        u32_le value{};
    };
    static_assert(sizeof(IoctlGetSyncpoint) == 8, "IocGetIdParams has wrong size");

    struct IoctlGetWaitbase {
        u32_le unknown{}; // seems to be ignored? Nintendo added this
        u32_le value{};
    };
    static_assert(sizeof(IoctlGetWaitbase) == 0x8, "IoctlGetWaitbase is incorrect size");

    struct IoctlMapBuffer {
        u32_le num_entries{};
        u32_le data_address{}; // Ignored by the driver.
        u32_le attach_host_ch_das{};
    };
    static_assert(sizeof(IoctlMapBuffer) == 0x0C, "IoctlMapBuffer is incorrect size");

    struct IocGetIdParams {
        // Input
        u32_le param{};
        // Output
        u32_le value{};
    };
    static_assert(sizeof(IocGetIdParams) == 8, "IocGetIdParams has wrong size");

    // Used for mapping and unmapping command buffers
    struct MapBufferEntry {
        u32_le map_handle{};
        u32_le map_address{};
    };
    static_assert(sizeof(IoctlMapBuffer) == 0x0C, "IoctlMapBuffer is incorrect size");

    /// Ioctl command implementations
    NvResult SetNVMAPfd(const std::vector<u8>& input);
    NvResult Submit(const std::vector<u8>& input, std::vector<u8>& output);
    NvResult GetSyncpoint(const std::vector<u8>& input, std::vector<u8>& output);
    NvResult GetWaitbase(const std::vector<u8>& input, std::vector<u8>& output);
    NvResult MapBuffer(const std::vector<u8>& input, std::vector<u8>& output);
    NvResult UnmapBuffer(const std::vector<u8>& input, std::vector<u8>& output);
    NvResult SetSubmitTimeout(const std::vector<u8>& input, std::vector<u8>& output);

    std::optional<BufferMap> FindBufferMap(GPUVAddr gpu_addr) const;
    void AddBufferMap(GPUVAddr gpu_addr, std::size_t size, VAddr cpu_addr, bool is_allocated);
    std::optional<std::size_t> RemoveBufferMap(GPUVAddr gpu_addr);

    s32_le nvmap_fd{};
    u32_le submit_timeout{};
    std::shared_ptr<nvmap> nvmap_dev;

    // This is expected to be ordered, therefore we must use a map, not unordered_map
    std::map<GPUVAddr, BufferMap> buffer_mappings;
};
}; // namespace Service::Nvidia::Devices
