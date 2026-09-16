#include <aspose/pdf/facades/form_editor.hpp>

#include <utility>

#include <aspose/pdf/document.hpp>
#include <aspose/pdf/rectangle.hpp>
#include <aspose/pdf/forms/barcode_field.hpp>
#include <aspose/pdf/forms/button_field.hpp>
#include <aspose/pdf/forms/checkbox_field.hpp>
#include <aspose/pdf/forms/combo_box_field.hpp>
#include <aspose/pdf/forms/date_field.hpp>
#include <aspose/pdf/forms/field.hpp>
#include <aspose/pdf/forms/form.hpp>
#include <aspose/pdf/forms/list_box_field.hpp>
#include <aspose/pdf/forms/number_field.hpp>
#include <aspose/pdf/forms/radio_button_field.hpp>
#include <aspose/pdf/forms/signature_field.hpp>
#include <aspose/pdf/forms/text_box_field.hpp>

namespace Aspose::Pdf::Facades {

namespace {

// Build the concrete Field for a requested FieldType at the given rect.
// Returns nullptr for types with no simple single-rect construction
// (InvalidNameOrType).
std::unique_ptr<Aspose::Pdf::Forms::Field> MakeField(
    FieldType type, Aspose::Pdf::Document& doc,
    const Aspose::Pdf::Rectangle& rect) {
    using namespace Aspose::Pdf::Forms;
    switch (type) {
        case FieldType::Text:
        case FieldType::MultiLineText:
            return std::make_unique<TextBoxField>(doc, rect);
        case FieldType::ComboBox:
            return std::make_unique<ComboBoxField>(doc, rect);
        case FieldType::ListBox:
            return std::make_unique<ListBoxField>(doc, rect);
        case FieldType::Radio:
            // RadioButtonField has no single-rect ctor (options carry
            // their own rects); construct field-only.
            return std::make_unique<RadioButtonField>(doc);
        case FieldType::CheckBox:
            return std::make_unique<CheckboxField>(doc, rect);
        case FieldType::PushButton:
        case FieldType::Image:
            return std::make_unique<ButtonField>(doc, rect);
        case FieldType::Barcode:
            return std::make_unique<BarcodeField>(doc, rect);
        case FieldType::Signature:
            return std::make_unique<SignatureField>(doc, rect);
        case FieldType::Numeric:
            return std::make_unique<NumberField>(doc, rect);
        case FieldType::DateTime:
            return std::make_unique<DateField>(doc, rect);
        case FieldType::InvalidNameOrType:
        default:
            return nullptr;
    }
}

}  // namespace

FormEditor::FormEditor(const std::string& srcFileName,
                       const std::string& destFileName)
    : src_file_(srcFileName), dest_file_(destFileName) {}

FormEditor::FormEditor(Aspose::Pdf::Document& document) {
    BindPdf(document);
}

FormEditor::FormEditor(Aspose::Pdf::Document& document,
                       const std::string& destFileName)
    : dest_file_(destFileName) {
    BindPdf(document);
}

FormEditor::~FormEditor() = default;

Aspose::Pdf::Document* FormEditor::EnsureDoc() {
    if (document_ == nullptr && !src_file_.empty()) {
        SaveableFacade::BindPdf(src_file_);  // loads + owns + sets document_
    }
    return document_;
}

void FormEditor::Save() {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc != nullptr && !dest_file_.empty()) {
        doc->Save(dest_file_);
    }
}

// ===== Field add / remove (real) =============================================

bool FormEditor::AddField(FieldType fieldType, const std::string& fieldName,
                          int pageNum, float llx, float lly, float urx,
                          float ury) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) {
        return false;
    }
    auto field = MakeField(fieldType, *doc,
                           Aspose::Pdf::Rectangle(llx, lly, urx, ury, true));
    if (!field) {
        return false;
    }
    doc->Form().Add(*field, fieldName, pageNum);
    owned_fields_.push_back(std::move(field));
    return true;
}

bool FormEditor::AddField(FieldType fieldType, const std::string& fieldName,
                          const std::string& /*fieldId*/, int pageNum,
                          float llx, float lly, float urx, float ury) {
    return AddField(fieldType, fieldName, pageNum, llx, lly, urx, ury);
}

void FormEditor::RemoveField(const std::string& fieldName) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc != nullptr) {
        doc->Form().Delete(fieldName);
    }
}

// ===== Field configuration ===================================================

bool FormEditor::SetFieldAttribute(const std::string& fieldName,
                                   PropertyFlag flag) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return false;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return false;
    auto* field = dynamic_cast<Forms::Field*>(annot);
    if (field == nullptr) return false;
    switch (flag) {
        case PropertyFlag::ReadOnly:
            field->ReadOnly(true);
            return true;
        case PropertyFlag::Required:
            field->Required(true);
            return true;
        case PropertyFlag::NoExport:
            field->Exportable(false);
            return true;
        case PropertyFlag::InvalidFlag:
        default:
            return false;
    }
}

bool FormEditor::SetFieldAppearance(
    const std::string& fieldName,
    Aspose::Pdf::Annotations::AnnotationFlags flag) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return false;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return false;
    annot->Flags(flag);
    return true;
}

Aspose::Pdf::Annotations::AnnotationFlags FormEditor::GetFieldAppearance(
    const std::string& fieldName) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return Aspose::Pdf::Annotations::AnnotationFlags{};
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return Aspose::Pdf::Annotations::AnnotationFlags{};
    return annot->Flags();
}

bool FormEditor::SetSubmitFlag(const std::string& /*fieldName*/,
                               SubmitFormFlag /*flag*/) {
    return false;
}

bool FormEditor::SetSubmitUrl(const std::string& /*fieldName*/,
                              const std::string& /*url*/) {
    return false;
}

bool FormEditor::SetFieldLimit(const std::string& fieldName, int fieldLimit) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return false;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return false;
    if (auto* textBox = dynamic_cast<Forms::TextBoxField*>(annot)) {
        textBox->MaxLen(fieldLimit);
        return true;
    }
    return false;
}

bool FormEditor::SetFieldCombNumber(const std::string& fieldName,
                                    int combNumber) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return false;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return false;
    if (auto* textBox = dynamic_cast<Forms::TextBoxField*>(annot)) {
        textBox->ForceCombs(true);
        textBox->MaxLen(combNumber);
        return true;
    }
    return false;
}

bool FormEditor::MoveField(const std::string& fieldName,
                           float llx, float lly, float urx, float ury) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return false;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return false;
    annot->Rect(Aspose::Pdf::Rectangle(llx, lly, urx, ury, true));
    return true;
}

bool FormEditor::SetFieldScript(const std::string& /*fieldName*/,
                                const std::string& /*script*/) {
    return false;
}

bool FormEditor::AddFieldScript(const std::string& /*fieldName*/,
                                const std::string& /*script*/) {
    return false;
}

bool FormEditor::Single2Multiple(const std::string& fieldName) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return false;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return false;
    if (auto* textBox = dynamic_cast<Forms::TextBoxField*>(annot)) {
        textBox->Multiline(true);
        return true;
    }
    return false;
}

bool FormEditor::SetFieldAlignment(const std::string& fieldName,
                                   int alignment) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return false;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return false;
    switch (alignment) {
        case FormFieldFacade::AlignLeft:
            annot->TextHorizontalAlignment(Aspose::Pdf::HorizontalAlignment::Left);
            annot->Alignment(Aspose::Pdf::Annotations::TextAlignment::Left);
            return true;
        case FormFieldFacade::AlignCenter:
            annot->TextHorizontalAlignment(Aspose::Pdf::HorizontalAlignment::Center);
            annot->Alignment(Aspose::Pdf::Annotations::TextAlignment::Center);
            return true;
        case FormFieldFacade::AlignRight:
            annot->TextHorizontalAlignment(Aspose::Pdf::HorizontalAlignment::Right);
            annot->Alignment(Aspose::Pdf::Annotations::TextAlignment::Right);
            return true;
        case FormFieldFacade::AlignJustified:
            annot->TextHorizontalAlignment(Aspose::Pdf::HorizontalAlignment::Justify);
            return true;
        default:
            return false;
    }
}

bool FormEditor::SetFieldAlignmentV(const std::string& fieldName,
                                    int alignment) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return false;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return false;
    if (auto* textBox = dynamic_cast<Forms::TextBoxField*>(annot)) {
        switch (alignment) {
            case FormFieldFacade::AlignTop:
                textBox->TextVerticalAlignment(Aspose::Pdf::VerticalAlignment::Top);
                return true;
            case FormFieldFacade::AlignMiddle:
                textBox->TextVerticalAlignment(Aspose::Pdf::VerticalAlignment::Center);
                return true;
            case FormFieldFacade::AlignBottom:
                textBox->TextVerticalAlignment(Aspose::Pdf::VerticalAlignment::Bottom);
                return true;
            default:
                return false;
        }
    }
    return false;
}

void FormEditor::ResetFacade() { facade_.Reset(); }
void FormEditor::ResetInnerFacade() { facade_.Reset(); }

void FormEditor::CopyInnerField(const std::string& fieldName,
                                const std::string& newFieldName, int pageNum) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return;
    auto rect = annot->Rect();
    auto field = MakeField(FieldType::Text, *doc, rect);
    if (field) {
        doc->Form().Add(*field, newFieldName, pageNum);
        owned_fields_.push_back(std::move(field));
    }
}

void FormEditor::CopyInnerField(const std::string& fieldName,
                                const std::string& newFieldName, int pageNum,
                                float offsetX, float offsetY) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return;
    auto rect = annot->Rect();
    Aspose::Pdf::Rectangle offset_rect(
        rect.LLX() + offsetX, rect.LLY() + offsetY,
        rect.URX() + offsetX, rect.URY() + offsetY, true);
    auto field = MakeField(FieldType::Text, *doc, offset_rect);
    if (field) {
        doc->Form().Add(*field, newFieldName, pageNum);
        owned_fields_.push_back(std::move(field));
    }
}

void FormEditor::CopyOuterField(const std::string& fieldName,
                                const std::string& newFieldName) {
    CopyInnerField(fieldName, newFieldName, 1);
}

void FormEditor::CopyOuterField(const std::string& fieldName,
                                const std::string& newFieldName, int pageNum) {
    CopyInnerField(fieldName, newFieldName, pageNum);
}

void FormEditor::CopyOuterField(const std::string& fieldName,
                                const std::string& newFieldName, int pageNum,
                                float offsetX, float offsetY) {
    CopyInnerField(fieldName, newFieldName, pageNum, offsetX, offsetY);
}

void FormEditor::DecorateField(const std::string& fieldName) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return;
    if (facade_.Alignment() != FormFieldFacade::AlignUndefined) {
        SetFieldAlignment(fieldName, facade_.Alignment());
    }
    if (facade_.BorderWidth() != FormFieldFacade::BorderWidthUndefined) {
        annot->Border().Width(static_cast<int>(facade_.BorderWidth()));
    }
}

void FormEditor::DecorateField(FieldType fieldType) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return;
    for (auto* field : doc->Form().Fields()) {
        if (field == nullptr) continue;
        bool match = false;
        switch (fieldType) {
            case FieldType::Text:
            case FieldType::MultiLineText:
                match = (dynamic_cast<Forms::TextBoxField*>(field) != nullptr);
                break;
            case FieldType::ComboBox:
                match = (dynamic_cast<Forms::ComboBoxField*>(field) != nullptr);
                break;
            case FieldType::ListBox:
                match = (dynamic_cast<Forms::ListBoxField*>(field) != nullptr);
                break;
            case FieldType::Radio:
                match = (dynamic_cast<Forms::RadioButtonField*>(field) != nullptr);
                break;
            case FieldType::CheckBox:
                match = (dynamic_cast<Forms::CheckboxField*>(field) != nullptr);
                break;
            case FieldType::PushButton:
            case FieldType::Image:
                match = (dynamic_cast<Forms::ButtonField*>(field) != nullptr);
                break;
            case FieldType::Barcode:
                match = (dynamic_cast<Forms::BarcodeField*>(field) != nullptr);
                break;
            case FieldType::Signature:
                match = (dynamic_cast<Forms::SignatureField*>(field) != nullptr);
                break;
            case FieldType::Numeric:
                match = (dynamic_cast<Forms::NumberField*>(field) != nullptr);
                break;
            case FieldType::DateTime:
                match = (dynamic_cast<Forms::DateField*>(field) != nullptr);
                break;
            default:
                break;
        }
        if (match) {
            DecorateField(field->PartialName());
        }
    }
}

void FormEditor::DecorateField() {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return;
    for (auto* field : doc->Form().Fields()) {
        if (field != nullptr) {
            DecorateField(field->PartialName());
        }
    }
}

void FormEditor::RenameField(const std::string& fieldName,
                             const std::string& newFieldName) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return;
    if (auto* field = dynamic_cast<Forms::Field*>(annot)) {
        field->PartialName(newFieldName);
    }
}

// v1 stub — removing a field's /A action needs action storage on
// WidgetAnnotation/Field, which v1 does not carry (like SetSubmitUrl /
// SetFieldScript above).
void FormEditor::RemoveFieldAction(const std::string& /*fieldName*/) {}

void FormEditor::AddSubmitBtn(const std::string& fieldName, int pageNum,
                              const std::string& buttonName,
                              const std::string& /*url*/,
                              float llx, float lly, float urx, float ury) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return;
    auto btn = std::make_unique<Forms::ButtonField>(
        *doc, Aspose::Pdf::Rectangle(llx, lly, urx, ury, true));
    btn->PartialName(fieldName);
    btn->AlternateName(buttonName);
    doc->Form().Add(*btn, fieldName, pageNum);
    owned_fields_.push_back(std::move(btn));
}

void FormEditor::AddListItem(const std::string& fieldName,
                             const std::string& item) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return;
    if (auto* choice = dynamic_cast<Forms::ChoiceField*>(annot)) {
        choice->AddOption(item);
    }
}

void FormEditor::AddListItem(const std::string& fieldName,
                             const std::vector<std::string>& items) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return;
    if (auto* choice = dynamic_cast<Forms::ChoiceField*>(annot)) {
        for (const auto& item : items) {
            choice->AddOption(item);
        }
    }
}

void FormEditor::DelListItem(const std::string& fieldName,
                             const std::string& item) {
    Aspose::Pdf::Document* doc = EnsureDoc();
    if (doc == nullptr) return;
    auto* annot = doc->Form()[fieldName];
    if (annot == nullptr) return;
    if (auto* choice = dynamic_cast<Forms::ChoiceField*>(annot)) {
        choice->DeleteOption(item);
    }
}

// ===== Properties ============================================================

const std::string& FormEditor::SrcFileName() const { return src_file_; }
void FormEditor::SrcFileName(std::string value) {
    src_file_ = std::move(value);
}
const std::string& FormEditor::DestFileName() const { return dest_file_; }
void FormEditor::DestFileName(std::string value) {
    dest_file_ = std::move(value);
}
void FormEditor::ConvertTo(Aspose::Pdf::PdfFormat value) {
    convert_to_ = value;
}
const std::vector<std::string>& FormEditor::Items() const { return items_; }
void FormEditor::Items(std::vector<std::string> value) {
    items_ = std::move(value);
}
const std::vector<std::vector<std::string>>& FormEditor::ExportItems() const {
    return export_items_;
}
void FormEditor::ExportItems(std::vector<std::vector<std::string>> value) {
    export_items_ = std::move(value);
}
const FormFieldFacade& FormEditor::Facade() const { return facade_; }
void FormEditor::Facade(FormFieldFacade value) { facade_ = std::move(value); }
float FormEditor::RadioGap() const noexcept { return radio_gap_; }
void FormEditor::RadioGap(float value) noexcept { radio_gap_ = value; }
bool FormEditor::RadioHoriz() const noexcept { return radio_horiz_; }
void FormEditor::RadioHoriz(bool value) noexcept { radio_horiz_ = value; }
double FormEditor::RadioButtonItemSize() const noexcept {
    return radio_button_item_size_;
}
void FormEditor::RadioButtonItemSize(double value) noexcept {
    radio_button_item_size_ = value;
}
SubmitFormFlag FormEditor::SubmitFlag() const noexcept { return submit_flag_; }
void FormEditor::SubmitFlag(SubmitFormFlag value) noexcept {
    submit_flag_ = value;
}

}  // namespace Aspose::Pdf::Facades
