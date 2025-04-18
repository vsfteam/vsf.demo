// refer to https://github.com/abhijitnandy2011/SDL-Browser-litehtml

// Impl

#include "SDLContainer.h"

// std
#include <math.h>
#include <iostream>
#include <wchar>

#define litehtml_container_trace        vsf_trace_debug
//#define litehtml_container_trace(...)

using namespace litehtml;

SDLContainer::SDLContainer(int width, int height)
{
    m_width = width;
    m_height = height;
    m_clip.w = 0;
    m_clip.h = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return;
    }
    m_window = SDL_CreateWindow("litehtml", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_SHOWN);
    if (NULL == m_window) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        goto deinit_and_failure;
    }
    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    if (NULL == m_renderer) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        goto destroy_window_and_failure;
    }
    SDL_SetRenderDrawColor(m_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(m_renderer);

    TTF_Init();
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    return;

destroy_renderer_and_failure:
    SDL_DestroyRenderer(m_renderer);
    m_renderer = NULL;
destroy_window_and_failure:
    SDL_DestroyWindow(m_window);
    m_window = NULL;
deinit_and_failure:
    SDL_Quit();
failure:
    return;
}

SDLContainer::~SDLContainer(void)
{
    while (!m_fonts.empty()) {
        auto it = m_fonts.begin();
        TTF_CloseFont(it->second);
        m_fonts.erase(it);
    }

    while (!m_images.empty()) {
        auto it = m_images.begin();
        SDL_FreeSurface(it->second);
        m_images.erase(it);
    }

    if (m_renderer != NULL) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = NULL;
    }
    if (m_window != NULL) {
        SDL_DestroyWindow(m_window);
        m_window = NULL;
    }
    SDL_Quit();
}

litehtml::uint_ptr SDLContainer::create_font(const litehtml::font_description& descr, const litehtml::document* doc, litehtml::font_metrics* fm)
{
    std::string fontKey = descr.hash();
    TTF_Font* font;

    if (m_fonts[fontKey]) {
        font = m_fonts[fontKey];
    } else {
        std::string fontPath = "/usr/share/font/truetype/" + descr.family + "-Bold.ttf";
        font = TTF_OpenFont(fontPath.c_str(), descr.size);
        m_fonts[fontKey] = font;
    }

    if(font == nullptr) {
        return 0;
    }

    int ttfStyle = TTF_STYLE_NORMAL;

    if(descr.style == font_style_italic) {
        ttfStyle = ttfStyle | TTF_STYLE_ITALIC;
    }

    if(descr.decoration_line & text_decoration_line_line_through) {
        ttfStyle = ttfStyle | TTF_STYLE_STRIKETHROUGH;
    }

    if((descr.decoration_line & text_decoration_line_underline) != 0) {
        ttfStyle = ttfStyle | TTF_STYLE_UNDERLINE;
    }

    if(descr.weight >= 700) {
        ttfStyle = ttfStyle | TTF_STYLE_BOLD;
    }

    TTF_SetFontStyle(font, ttfStyle);

    int iWidth = 0, iHeight = 0;
    TTF_SizeText(font, "x", &iWidth, &iHeight);

    if (fm) {
        // fixme cache this
        fm->ascent    = TTF_FontAscent(font);
        fm->descent   = TTF_FontDescent(font);
        fm->height    = TTF_FontHeight(font);
        fm->x_height  = iWidth;
        fm->draw_spaces = descr.style == font_style_italic || descr.decoration_line;
    }

    return (uint_ptr) font;
}

void SDLContainer::delete_font(litehtml::uint_ptr hFont)
{
    TTF_Font* font = (TTF_Font*)hFont;
    if(font) {
        TTF_CloseFont(font);
    }
}

int SDLContainer::text_width(const char* text, litehtml::uint_ptr hFont)
{
    TTF_Font* font = (TTF_Font*)hFont;
    if(!font) {
        return 0;
    }

    int iWidth = 0, iHeight = 0;
    TTF_SizeUTF8(font, text, &iWidth, &iHeight);
    return iWidth;
}

void SDLContainer::draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos)
{
    SDL_Color sdlcolor={color.red, color.green, color.blue, color.alpha};
    SDL_Surface *info;
    TTF_Font* font = (TTF_Font*)hFont;

    if(!(info=TTF_RenderUTF8_Blended(font, text, sdlcolor))) {
        //handle error here, perhaps print TTF_GetError at least
    } else {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(m_renderer, info);
        SDL_Rect src = { 0, 0, info->w, info->h };
        SDL_Rect dst = { pos.x, pos.y, info->w, info->h };
        SDL_RenderCopy(m_renderer, texture, &src, &dst);
        SDL_DestroyTexture(texture);
    }

}

void SDLContainer::draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url)
{
    litehtml_container_trace("%s: %s %s\n", __FUNCTION__, base_url.c_str(), url.c_str());

    std::string imageKey = base_url;
    imageKey += url;

    SDL_Surface* image = m_images[imageKey];
    if (NULL == image) {
        vsf_trace_error("%s: no image loaded at key %s\n", __FUNCTION__, imageKey.c_str());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, image);
    if (NULL == texture) {
        vsf_trace_error("%s: fail to create texture from surface\n", __FUNCTION__);
        return;
    }
    SDL_Rect src = { 0, 0, image->w, image->h };
    SDL_Rect dst = { layer.origin_box.x, layer.origin_box.y, layer.origin_box.width, layer.origin_box.height };
    SDL_RenderCopy(m_renderer, texture, &src, &dst);
    SDL_DestroyTexture(texture);
}

void SDLContainer::draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color)
{
    SDL_Rect rect = {
        .x      = layer.border_box.x,
        .y      = layer.border_box.y,
        .w      = layer.border_box.width,
        .h      = layer.border_box.height,
    };
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(m_renderer, &r, &g, &b, &a);
    SDL_SetRenderDrawColor(m_renderer, color.red, color.green, color.blue, color.alpha);
    SDL_RenderDrawRect(m_renderer, &rect);
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
}

void SDLContainer::draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient)
{
    litehtml_container_trace("%s:\n", __FUNCTION__);
}

void SDLContainer::draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient)
{
    litehtml_container_trace("%s:\n", __FUNCTION__);
}

void SDLContainer::draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient)
{
    litehtml_container_trace("%s:\n", __FUNCTION__);
}

int SDLContainer::pt_to_px(int pt) const
{
    litehtml_container_trace("%s:\n", __FUNCTION__);
    return pt; // (int) round(pt * 125.0 / 72.0);
}

int SDLContainer::get_default_font_size() const
{
    return 16;
}

void SDLContainer::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
    litehtml_container_trace("%s:\n", __FUNCTION__);
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(m_renderer, &r, &g, &b, &a);

    if (!marker.image.empty())
    {
        /*std::wstring url;
        t_make_url(marker.image.c_str(), marker.baseurl, url);

        lock_images_cache();
        images_map::iterator img_i = m_images.find(url.c_str());
        if (img_i != m_images.end())
        {
            if (img_i->second)
            {
                draw_txdib((cairo_t*)hdc, img_i->second.get(), marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
            }
        }
        unlock_images_cache();*/
    }
    else
    {
        switch (marker.marker_type)
        {
        case litehtml::list_style_type_circle:
//            circleRGBA(m_renderer, marker.pos.x, marker.pos.y, marker.pos.width, marker.color.red, marker.color.green, marker.color.blue, marker.color.alpha);
            break;
        case litehtml::list_style_type_disc:
//            filledCircleRGBA(m_renderer, marker.pos.x, marker.pos.y, marker.pos.width, marker.color.red, marker.color.green, marker.color.blue, marker.color.alpha);
            break;
        case litehtml::list_style_type_square:
            SDL_Rect fillRect = { marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height };
            SDL_SetRenderDrawColor(m_renderer, marker.color.red, marker.color.green, marker.color.blue, marker.color.alpha);
            SDL_RenderFillRect(m_renderer, &fillRect);
            break;
        }
    }

    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
}

void SDLContainer::load_image(const char* src, const char* baseurl, bool redraw_on_ready)
{
    litehtml_container_trace("%s: %s %s\n", __FUNCTION__, baseurl ? baseurl : "", src);

    std::string imageKey = baseurl;
    imageKey += src;

    if (!m_images[imageKey]) {
        litehtml::string image_buff;
        litehtml::string url = src;

        if (url.find("//", 0) == 0) {
            url.insert(0, "https:");
        }

        if (request_target(image_buff, url, baseurl)) {
            SDL_RWops *rw = SDL_RWFromMem((void *)image_buff.c_str(), image_buff.length());
            m_images[imageKey] = IMG_Load_RW(rw, 0);
        } else {
            vsf_trace_error("%s: fail to download image at %s %s\n", __FUNCTION__, NULL == baseurl ? "" : baseurl, src);
        }
    }

    SDL_Surface* image = m_images[imageKey];
    if (NULL == image) {
        vsf_trace_error("%s: fail to load image at %s %s\n", __FUNCTION__, NULL == baseurl ? "" : baseurl, src);
    }
}

void SDLContainer::get_image_size(const char* src, const char* baseurl, litehtml::size& sz)
{
    litehtml_container_trace("%s: %s %s\n", __FUNCTION__, baseurl ? baseurl : "", src);

    std::string imageKey = baseurl;
    imageKey += src;

    SDL_Surface* image = m_images[imageKey];
    if (image != NULL) {
        sz.width = image->w;
        sz.height = image->h;
    } else {
        vsf_trace_error("%s: no image loaded at key %s\n", __FUNCTION__, imageKey.c_str());
        sz.width = 0;
        sz.height = 0;
    }
}

void SDLContainer::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
{
    if (borders.top.width != 0 && borders.top.style > litehtml::border_style_hidden) {
        Uint8 r, g, b, a;
        SDL_GetRenderDrawColor(m_renderer, &r, &g, &b, &a);
        SDL_Rect fillRect = { draw_pos.x, draw_pos.y, draw_pos.width, draw_pos.height };
        SDL_SetRenderDrawColor(m_renderer, borders.top.color.red, borders.top.color.green, borders.top.color.blue, borders.top.color.alpha);
        SDL_RenderDrawRect(m_renderer, &fillRect);       
        SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    }
}

void SDLContainer::transform_text(litehtml::string& text, litehtml::text_transform tt)
{
    litehtml_container_trace("%s:\n", __FUNCTION__);
}

void SDLContainer::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius)
{
    m_clip.x = pos.x;
    m_clip.y = pos.y;
    m_clip.w = pos.width;
    m_clip.h = pos.height;
}

void SDLContainer::del_clip()
{
    m_clip.w = 0;
    m_clip.h = 0;
}

void SDLContainer::on_anchor_click(const char* url, const litehtml::element::ptr& el) 
{
    litehtml_container_trace("%s:\n", __FUNCTION__);
}

void SDLContainer::on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event)
{
    litehtml_container_trace("%s:\n", __FUNCTION__);
}

void SDLContainer::get_viewport(litehtml::position& viewport) const
{
    viewport.width = m_width;
    viewport.height = m_height;
    viewport.x = viewport.y = 0;
}


void SDLContainer::set_cursor(const char* cursor) 
{
    litehtml_container_trace("%s:\n", __FUNCTION__);

}

void SDLContainer::import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) 
{
    litehtml_container_trace("%s: %s \n", __FUNCTION__, url.c_str());
}

void SDLContainer::set_caption(const char* caption) 
{
    litehtml_container_trace("%s: %s\n", __FUNCTION__, caption);
}

void SDLContainer::set_base_url(const char* base_url) 
{
    litehtml_container_trace("%s: %s\n", __FUNCTION__, base_url);
}


const char* SDLContainer::get_default_font_name() const
{
    return "MiSans";
}

std::shared_ptr<litehtml::element>  SDLContainer::create_element(const char *tag_name,
                                    const litehtml::string_map &attributes,
                                    const std::shared_ptr<litehtml::document> &doc)
{
    return nullptr;
}

void SDLContainer::get_media_features(litehtml::media_features& media) const
{
    media.type          = litehtml::media_type_screen;
    media.width         = m_width;
    media.height        = m_height;
    media.device_width  = m_width;
    media.device_height = m_height;
    media.color         = 32;
    media.monochrome    = 0;
    media.color_index   = 0;
    media.resolution    = 96;
}

void SDLContainer::get_language(litehtml::string& language, litehtml::string& culture) const
{
    language = "en";
    culture = "";
}

void SDLContainer::link(const std::shared_ptr<litehtml::document> &ptr, const litehtml::element::ptr& el)
{
}

SDL_Renderer* SDLContainer::get_renderer()
{
    return m_renderer;
}