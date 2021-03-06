#include "chanmap.h"
#include <numeric>
#include <cassert>

namespace chanmap {

const char *getChannelName(uint32_t n)
{
    const char *tab[] = {
        "?","L","R","C","LFE","Ls","Rs","Lc","Rc","Cs",
        "Lsd","Rsd","Ts","Vhl","Vhc","Vhr","Tbl","Tbc","Tbr"
    };
    if (n <= 18) return tab[n];
    switch (n) {
    case 33: return "Rls";
    case 34: return "Rrs";
    case 35: return "Lw";
    case 36: return "Rw";
    }
    return "?";
}

std::string getChannelNames(const std::vector<uint32_t> &channels)
{
    const char *delim = "";
    std::string result;

    std::for_each(channels.begin(), channels.end(), [&](uint32_t c) {
        result.append(delim).append(getChannelName(c));
        delim = " ";
    });
    size_t lfe_count = std::count(channels.begin(), channels.end(), 4);
    size_t count = channels.size() - lfe_count;
    if (count <= 2 && lfe_count == 0)
        return count == 1 ? "Mono" : "Stereo";
    else
        return strutil::format("%u.%u (%s)",
                               static_cast<uint32_t>(count),
                               static_cast<uint32_t>(lfe_count),
                               result.c_str());
}

uint32_t getChannelMask(const std::vector<uint32_t>& channels)
{
    if (std::count_if(channels.begin(), channels.end(), [](uint32_t c) {
                      return c >= 33; }))
        throw std::runtime_error("Not supported channel layout");

    return std::accumulate(channels.begin(), channels.end(), 0,
                           [](uint32_t a, uint32_t c) {
                               return a | (1 << (c - 1));
                           });
}

void getChannels(uint32_t bitmap, std::vector<uint32_t> *result,
                 uint32_t limit)
{
    std::vector<uint32_t> channels;
    for (unsigned i = 0; i < 32 && channels.size() < limit; ++i) {
        if (bitmap & (1<<i))
            channels.push_back(i + 1);
    }
    result->swap(channels);
}

void getChannels(const AudioChannelLayout *acl, std::vector<uint32_t> *result)
{
    std::vector<uint32_t> channels;
    uint32_t bitmap = 0;
    const char *layout = 0;

    switch (acl->mChannelLayoutTag) {
    case kAudioChannelLayoutTag_UseChannelBitmap:
        bitmap = acl->mChannelBitmap; break;
    case kAudioChannelLayoutTag_UseChannelDescriptions:
    {
        const AudioChannelDescription *desc = acl->mChannelDescriptions;
        for (size_t i = 0; i < acl->mNumberChannelDescriptions; ++i)
            channels.push_back(desc[i].mChannelLabel);
        break;
    }
    /* 1ch */
    case kAudioChannelLayoutTag_Mono:
        layout = "\x2a"; break; /* kAudioChannelLabel_Mono */
    /* 1.1ch */
    case kAudioChannelLayoutTag_AC3_1_0_1:
        layout = "\x03\x04"; break;
    /* 2ch */
    case kAudioChannelLayoutTag_Stereo:
    case kAudioChannelLayoutTag_MatrixStereo: /* XXX: Actually Lt+Rt */
    case kAudioChannelLayoutTag_Binaural:
        layout = "\x01\x02"; break;
    /* 2.1ch */
    case kAudioChannelLayoutTag_DVD_4:
        layout = "\x01\x02\x04"; break;
    /* 3ch */
    case kAudioChannelLayoutTag_MPEG_3_0_A:
        layout = "\x01\x02\x03"; break;
    case kAudioChannelLayoutTag_AC3_3_0:
        layout = "\x01\x03\x02"; break;
    case kAudioChannelLayoutTag_MPEG_3_0_B:
        layout = "\x03\x01\x02"; break;
    case kAudioChannelLayoutTag_ITU_2_1:
        layout = "\x01\x02\x09"; break;
    /* 3.1ch */
    case kAudioChannelLayoutTag_DVD_10:
        layout = "\x01\x02\x03\x04"; break;
    case kAudioChannelLayoutTag_AC3_3_0_1:
        layout = "\x01\x03\x02\x04"; break;
    case kAudioChannelLayoutTag_DVD_5:
        layout = "\x01\x02\x04\x09"; break;
    case kAudioChannelLayoutTag_AC3_2_1_1:
        layout = "\x01\x02\x09\x04"; break;
    /* 4ch */
    case kAudioChannelLayoutTag_Quadraphonic:
    case kAudioChannelLayoutTag_ITU_2_2:
        layout = "\x01\x02\x05\x06"; break;
    case kAudioChannelLayoutTag_MPEG_4_0_A:
        layout = "\x01\x02\x03\x09"; break;
    case kAudioChannelLayoutTag_MPEG_4_0_B:
        layout = "\x03\x01\x02\x09"; break;
    case kAudioChannelLayoutTag_AC3_3_1:
        layout = "\x01\x03\x02\x09"; break;
    /* 4.1ch */
    case kAudioChannelLayoutTag_DVD_6:
        layout = "\x01\x02\x04\x05\x06"; break;
    case kAudioChannelLayoutTag_DVD_18:
        layout = "\x01\x02\x05\x06\x04"; break;
    case kAudioChannelLayoutTag_DVD_11:
        layout = "\x01\x02\x03\x04\x09"; break;
    case kAudioChannelLayoutTag_AC3_3_1_1:
        layout = "\x01\x03\x02\x09\x04"; break;
    /* 5ch */
    case kAudioChannelLayoutTag_MPEG_5_0_A:
        layout = "\x01\x02\x03\x05\x06"; break;
    case kAudioChannelLayoutTag_Pentagonal:
    case kAudioChannelLayoutTag_MPEG_5_0_B:
        layout = "\x01\x02\x05\x06\x03"; break;
    case kAudioChannelLayoutTag_MPEG_5_0_C:
        layout = "\x01\x03\x02\x05\x06"; break;
    case kAudioChannelLayoutTag_MPEG_5_0_D:
        layout = "\x03\x01\x02\x05\x06"; break;
    /* 5.1ch */
    case kAudioChannelLayoutTag_MPEG_5_1_A:
        layout = "\x01\x02\x03\x04\x05\x06"; break;
    case kAudioChannelLayoutTag_MPEG_5_1_B:
        layout = "\x01\x02\x05\x06\x03\x04"; break;
    case kAudioChannelLayoutTag_MPEG_5_1_C:
        layout = "\x01\x03\x02\x05\x06\x04"; break;
    case kAudioChannelLayoutTag_MPEG_5_1_D:
        layout = "\x03\x01\x02\x05\x06\x04"; break;
    /* 6ch */
    case kAudioChannelLayoutTag_Hexagonal:
    case kAudioChannelLayoutTag_AudioUnit_6_0:
        layout = "\x01\x02\x05\x06\x03\x09"; break;
    case kAudioChannelLayoutTag_AAC_6_0:
        layout = "\x03\x01\x02\x05\x06\x09"; break;
    /* 6.1ch */
    case kAudioChannelLayoutTag_MPEG_6_1_A:
        layout = "\x01\x02\x03\x04\x05\x06\x09"; break;
    case kAudioChannelLayoutTag_AAC_6_1:
        layout = "\x03\x01\x02\x05\x06\x09\x04"; break;
    /* 7ch */
    case kAudioChannelLayoutTag_AudioUnit_7_0:
        layout = "\x01\x02\x05\x06\x03\x21\x22"; break;
    case kAudioChannelLayoutTag_AudioUnit_7_0_Front:
        layout = "\x01\x02\x05\x06\x03\x07\x08"; break;
    case kAudioChannelLayoutTag_AAC_7_0:
        layout = "\x03\x01\x02\x05\x06\x21\x22"; break;
    /* 7.1ch */
    case kAudioChannelLayoutTag_MPEG_7_1_A:
        layout = "\x01\x02\x03\x04\x05\x06\x07\x08"; break;
    case kAudioChannelLayoutTag_MPEG_7_1_B:
        layout = "\x03\x07\x08\x01\x02\x05\x06\x04"; break;
    case kAudioChannelLayoutTag_MPEG_7_1_C:
        layout = "\x01\x02\x03\x04\x05\x06\x21\x22"; break;
    case kAudioChannelLayoutTag_Emagic_Default_7_1:
        layout = "\x01\x02\x05\x06\x03\x04\x07\x08"; break;
    /* 8ch */
    case kAudioChannelLayoutTag_Octagonal:
        /* XXX: actually the last two are Left Wide/Right Wide */
        layout = "\x01\x02\x05\x06\x03\x09\x0A\x0B"; break;
    case kAudioChannelLayoutTag_AAC_Octagonal:
        layout = "\x03\x01\x02\x05\x06\x21\x22\x09"; break;
    default:
        throw std::runtime_error("Unsupported channel layout");
    }

    if (bitmap)
        getChannels(bitmap, &channels);
    else if (layout)
        while (*layout) channels.push_back(*layout++);

    result->swap(channels);
}

void convertFromAppleLayout(std::vector<uint32_t> *channels)
{
    if (!std::count_if(channels->begin(), channels->end(),
                       [](uint32_t c) {
                           return c > 18;
                       }))
        return;

    std::vector<uint32_t> v(channels->size());
    std::transform(channels->begin(), channels->end(), v.begin(),
                   [](uint32_t x) ->uint32_t {
                       switch (x) {
                       case kAudioChannelLabel_Mono:
                           return kAudioChannelLabel_Center;
                       case kAudioChannelLabel_HeadphonesLeft:
                           return kAudioChannelLabel_Left;
                       case kAudioChannelLabel_HeadphonesRight:
                           return kAudioChannelLabel_Right;
                       }
                       return x;
                   });

    size_t back = std::count(v.begin(), v.end(), 5) +
                  std::count(v.begin(), v.end(), 6);
    size_t side = std::count(v.begin(), v.end(), 10) +
                  std::count(v.begin(), v.end(), 11);

    std::transform(v.begin(), v.end(), v.begin(), [&](uint32_t c) -> uint32_t {
                        switch (c) {
                        case kAudioChannelLabel_LeftSurround:
                        case kAudioChannelLabel_RightSurround:
                            if (!side) c += 5;
                            break;
                        case kAudioChannelLabel_RearSurroundLeft:
                        case kAudioChannelLabel_RearSurroundRight:
                            if (!back || !side) c -= 28;
                            break;
                        };
                        return c;
                   });
    channels->swap(v);
}

void getMappingToUSBOrder(const std::vector<uint32_t> &channels,
                          std::vector<uint32_t> *result)
{
    std::vector<uint32_t> index(channels.size());
    unsigned i = 0;
    std::generate(index.begin(), index.end(), [&](){ return ++i; });
    std::sort(index.begin(), index.end(), [&](uint32_t a, uint32_t b) {
                  return channels[a-1] < channels[b-1];
              });
    result->swap(index);
}

uint32_t defaultChannelMask(const uint32_t nchannels)
{
    static const uint32_t tab[] = {
        0x4, // FC
        0x3, // FL FR
        0x7, // FL FR FC
        0x33, // FL FR BL BR
        0x37, // FL FR FC BL BR
        0x3f, // FL FR FC LFE BL BR
        0x13f, // FL FR FC LFE BL BR BC
        0x63f // FL FR FC LFE BL BR SL SR
    };
    return tab[nchannels - 1];
}

uint32_t AACLayoutFromBitmap(uint32_t bitmap)
{
    if ((bitmap & 0x600) == 0x600 && (bitmap & 0x30) == 0) {
        bitmap &= ~0x600;
        bitmap |= 0x30;
    }
    switch (bitmap) {
    case 0x4:   return kAudioChannelLayoutTag_Mono;
    case 0x3:   return kAudioChannelLayoutTag_Stereo;
    case 0x7:   return kAudioChannelLayoutTag_AAC_3_0;
    case 0x33:  return kAudioChannelLayoutTag_Quadraphonic;
    case 0x107: return kAudioChannelLayoutTag_AAC_4_0;
    case 0x1c4: return kAudioChannelLayoutTag_AAC_4_0;
    case 0x37:  return kAudioChannelLayoutTag_AAC_5_0;
    case 0x3f:  return kAudioChannelLayoutTag_AAC_5_1;
    case 0x137: return kAudioChannelLayoutTag_AAC_6_0;
    case 0x13f: return kAudioChannelLayoutTag_AAC_6_1;
    case 0x637: return kAudioChannelLayoutTag_AAC_7_0;
    case 0xff:  return kAudioChannelLayoutTag_AAC_7_1;
    case 0x63f: return kAudioChannelLayoutTag_AAC_7_1; /* XXX */
    case 0x737: return kAudioChannelLayoutTag_AAC_Octagonal;
    }
    throw std::runtime_error("No channel mapping to AAC defined");
}

void getMappingToAAC(uint32_t bitmap, std::vector<uint32_t> *result)
{
    AudioChannelLayout aacLayout = { 0 };
    aacLayout.mChannelLayoutTag = AACLayoutFromBitmap(bitmap);

    std::vector<uint32_t> cs, mapping;
    getChannels(&aacLayout, &cs);
    /*
     * rewrite channels in pre-defined AAC channel layout to match with
     * input channel bitmap
     */
    std::transform(cs.begin(), cs.end(), cs.begin(),
                   [&](uint32_t c) -> uint32_t {
                        switch (c) {
                        case kAudioChannelLabel_Left:
                        case kAudioChannelLabel_Right:
                            if (bitmap == 0x63f)
                                c += 9;
                            else if (!(bitmap & 0x3) && (bitmap & 0xc))
                                c += 6;
                            break;
                        case kAudioChannelLabel_LeftCenter:
                        case kAudioChannelLabel_RightCenter:
                            if (bitmap == 0x63f)
                                c -= 6;
                            break;
                        case kAudioChannelLabel_LeftSurround:
                        case kAudioChannelLabel_RightSurround:
                            if (!(bitmap & 0x30) && (bitmap & 0x600))
                                c += 5;
                            break;
                        case kAudioChannelLabel_RearSurroundLeft:
                        case kAudioChannelLabel_RearSurroundRight:
                            c -= 18;
                            break;
                        }
                        return c;
                  });
    assert(getChannelMask(cs) == bitmap);
    getMappingToUSBOrder(cs, &mapping);
    getMappingToUSBOrder(mapping, &mapping);
    result->swap(mapping);
}

} // namespace

ChannelMapper::ChannelMapper(const std::shared_ptr<ISource> &source,
                             const std::vector<uint32_t> &chanmap,
                             uint32_t bitmap)
    : FilterBase(source)
{
    const AudioStreamBasicDescription &asbd = source->getSampleFormat();
    assert(chanmap.size() == asbd.mChannelsPerFrame);
    assert(chanmap.size() <= 8);

    for (size_t i = 0; i < chanmap.size(); ++i)
        m_chanmap.push_back(chanmap[i] - 1);
    if (bitmap) {
        for (unsigned i = 0; i < 32; ++i, bitmap >>= 1)
            if (bitmap & 1) m_layout.push_back(i + 1);
    } else {
        const std::vector<uint32_t> *orig = FilterBase::getChannels();
        if (orig)
            for (size_t i = 0; i < m_chanmap.size(); ++i)
                m_layout.push_back(orig->at(m_chanmap[i]));
    }
    switch (asbd.mBytesPerFrame / asbd.mChannelsPerFrame) {
    case 2:
        m_process = &ChannelMapper::process16; break;
    case 4:
        m_process = &ChannelMapper::process32; break;
    case 8:
        m_process = &ChannelMapper::process64; break;
    default:
        assert(0);
    }
}

template <typename T>
size_t ChannelMapper::processT(T *buffer, size_t nsamples)
{
    unsigned nchannels = source()->getSampleFormat().mChannelsPerFrame;
    const uint32_t *chanmap = &m_chanmap[0];
    T work[8], *bp = buffer;

    nsamples = source()->readSamples(buffer, nsamples);
    for (size_t i = 0; i < nsamples; ++i, bp += nchannels) {
        memcpy(work, bp, sizeof(T) * nchannels);
        switch (nchannels) {
        case 8: bp[7] = work[chanmap[7]];
        case 7: bp[6] = work[chanmap[6]];
        case 6: bp[5] = work[chanmap[5]];
        case 5: bp[4] = work[chanmap[4]];
        case 4: bp[3] = work[chanmap[3]];
        case 3: bp[2] = work[chanmap[2]];
        case 2: bp[1] = work[chanmap[1]];
        case 1: bp[0] = work[chanmap[0]];
        }
    }
    return nsamples;
}

size_t ChannelMapper::process16(void *buffer, size_t nsamples)
{
    return processT(static_cast<uint16_t *>(buffer), nsamples);
}

size_t ChannelMapper::process32(void *buffer, size_t nsamples)
{
    return processT(static_cast<uint32_t *>(buffer), nsamples);
}

size_t ChannelMapper::process64(void *buffer, size_t nsamples)
{
    return processT(static_cast<uint64_t *>(buffer), nsamples);
}
