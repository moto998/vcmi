/*
 * BattleInterfaceClasses.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "BattleInterfaceClasses.h"

#include "BattleInterface.h"
#include "BattleActionsController.h"
#include "BattleRenderer.h"
#include "BattleSiegeController.h"
#include "BattleFieldController.h"
#include "BattleStacksController.h"
#include "BattleWindow.h"

#include "../CGameInfo.h"
#include "../CMusicHandler.h"
#include "../CPlayerInterface.h"
#include "../CVideoHandler.h"
#include "../gui/CursorHandler.h"
#include "../gui/CGuiHandler.h"
#include "../gui/Shortcut.h"
#include "../gui/MouseButton.h"
#include "../gui/WindowHandler.h"
#include "../render/Canvas.h"
#include "../render/IImage.h"
#include "../render/IFont.h"
#include "../render/Graphics.h"
#include "../widgets/Buttons.h"
#include "../widgets/Images.h"
#include "../widgets/TextControls.h"
#include "../windows/CMessage.h"
#include "../windows/CSpellWindow.h"
#include "../render/CAnimation.h"
#include "../render/IRenderHandler.h"
#include "../adventureMap/CInGameConsole.h"

#include "../../CCallback.h"
#include "../../lib/CStack.h"
#include "../../lib/CConfigHandler.h"
#include "../../lib/CCreatureHandler.h"
#include "../../lib/gameState/InfoAboutArmy.h"
#include "../../lib/CGeneralTextHandler.h"
#include "../../lib/CTownHandler.h"
#include "../../lib/CHeroHandler.h"
#include "../../lib/StartInfo.h"
#include "../../lib/CondSh.h"
#include "../../lib/mapObjects/CGTownInstance.h"
#include "../../lib/networkPacks/PacksForClientBattle.h"
#include "../../lib/TextOperations.h"

void BattleConsole::showAll(Canvas & to)
{
	CIntObject::showAll(to);

	Point line1 (pos.x + pos.w/2, pos.y +  8);
	Point line2 (pos.x + pos.w/2, pos.y + 24);

	auto visibleText = getVisibleText();

	if(visibleText.size() > 0)
		to.drawText(line1, FONT_SMALL, Colors::WHITE, ETextAlignment::CENTER, visibleText[0]);

	if(visibleText.size() > 1)
		to.drawText(line2, FONT_SMALL, Colors::WHITE, ETextAlignment::CENTER, visibleText[1]);
}

std::vector<std::string> BattleConsole::getVisibleText()
{
	// high priority texts that hide battle log entries
	for(const auto & text : {consoleText, hoverText})
	{
		if (text.empty())
			continue;

		auto result = CMessage::breakText(text, pos.w, FONT_SMALL);

		if(result.size() > 2)
			result.resize(2);
		return result;
	}

	// log is small enough to fit entirely - display it as such
	if (logEntries.size() < 3)
		return logEntries;

	return { logEntries[scrollPosition - 1], logEntries[scrollPosition] };
}

std::vector<std::string> BattleConsole::splitText(const std::string &text)
{
	std::vector<std::string> lines;
	std::vector<std::string> output;

	boost::split(lines, text, boost::is_any_of("\n"));

	for(const auto & line : lines)
	{
		if (graphics->fonts[FONT_SMALL]->getStringWidth(text) < pos.w)
		{
			output.push_back(line);
		}
		else
		{
			std::vector<std::string> substrings = CMessage::breakText(line, pos.w, FONT_SMALL);
			output.insert(output.end(), substrings.begin(), substrings.end());
		}
	}
	return output;
}

bool BattleConsole::addText(const std::string & text)
{
	logGlobal->trace("CBattleConsole message: %s", text);

	auto newLines = splitText(text);

	logEntries.insert(logEntries.end(), newLines.begin(), newLines.end());
	scrollPosition = (int)logEntries.size()-1;
	redraw();
	return true;
}
void BattleConsole::scrollUp(ui32 by)
{
	if(scrollPosition > static_cast<int>(by))
		scrollPosition -= by;
	redraw();
}

void BattleConsole::scrollDown(ui32 by)
{
	if(scrollPosition + by < logEntries.size())
		scrollPosition += by;
	redraw();
}

BattleConsole::BattleConsole(std::shared_ptr<CPicture> backgroundSource, const Point & objectPos, const Point & imagePos, const Point &size)
	: scrollPosition(-1)
	, enteringText(false)
{
	OBJ_CONSTRUCTION_CAPTURING_ALL_NO_DISPOSE;
	pos += objectPos;
	pos.w = size.x;
	pos.h = size.y;

	background = std::make_shared<CPicture>(backgroundSource->getSurface(), Rect(imagePos, size), 0, 0 );
}

void BattleConsole::deactivate()
{
	if (enteringText)
		LOCPLINT->cingconsole->endEnteringText(false);

	CIntObject::deactivate();
}

void BattleConsole::setEnteringMode(bool on)
{
	consoleText.clear();

	if (on)
	{
		assert(enteringText == false);
		GH.startTextInput(pos);
	}
	else
	{
		assert(enteringText == true);
		GH.stopTextInput();
	}
	enteringText = on;
	redraw();
}

void BattleConsole::setEnteredText(const std::string & text)
{
	assert(enteringText == true);
	consoleText = text;
	redraw();
}

void BattleConsole::write(const std::string & Text)
{
	hoverText = Text;
	redraw();
}

void BattleConsole::clearIfMatching(const std::string & Text)
{
	if (hoverText == Text)
		clear();
}

void BattleConsole::clear()
{
	write({});
}

const CGHeroInstance * BattleHero::instance()
{
	return hero;
}

void BattleHero::tick(uint32_t msPassed)
{
	size_t groupIndex = static_cast<size_t>(phase);

	float timePassed = msPassed / 1000.f;

	flagCurrentFrame += currentSpeed * timePassed;
	currentFrame += currentSpeed * timePassed;

	if(flagCurrentFrame >= flagAnimation->size(0))
		flagCurrentFrame -= flagAnimation->size(0);

	if(currentFrame >= animation->size(groupIndex))
	{
		currentFrame -= animation->size(groupIndex);
		switchToNextPhase();
	}
}

void BattleHero::render(Canvas & canvas)
{
	size_t groupIndex = static_cast<size_t>(phase);

	auto flagFrame = flagAnimation->getImage(flagCurrentFrame, 0, true);
	auto heroFrame = animation->getImage(currentFrame, groupIndex, true);

	Point heroPosition = pos.center() - parent->pos.topLeft() - heroFrame->dimensions() / 2;
	Point flagPosition = pos.center() - parent->pos.topLeft() - flagFrame->dimensions() / 2;

	if(defender)
		flagPosition += Point(-4, -41);
	else
		flagPosition += Point(4, -41);

	canvas.draw(flagFrame, flagPosition);
	canvas.draw(heroFrame, heroPosition);
}

void BattleHero::pause()
{
	currentSpeed = 0.f;
}

void BattleHero::play()
{
	//H3 speed: 10 fps ( 100 ms per frame)
	currentSpeed = 10.f;
}

float BattleHero::getFrame() const
{
	return currentFrame;
}

void BattleHero::collectRenderableObjects(BattleRenderer & renderer)
{
	auto hex = defender ? BattleHex(GameConstants::BFIELD_WIDTH-1) : BattleHex(0);

	renderer.insert(EBattleFieldLayer::HEROES, hex, [this](BattleRenderer::RendererRef canvas)
	{
		render(canvas);
	});
}

void BattleHero::onPhaseFinished(const std::function<void()> & callback)
{
	phaseFinishedCallback = callback;
}

void BattleHero::setPhase(EHeroAnimType newPhase)
{
	nextPhase = newPhase;
	switchToNextPhase(); //immediately switch to next phase and then restore idling phase
	nextPhase = EHeroAnimType::HOLDING;
}

void BattleHero::heroLeftClicked()
{
	if(owner.actionsController->spellcastingModeActive()) //we are casting a spell
		return;

	if(!hero || !owner.makingTurn())
		return;

	if(owner.getBattle()->battleCanCastSpell(hero, spells::Mode::HERO) == ESpellCastProblem::OK) //check conditions
	{
		CCS->curh->set(Cursor::Map::POINTER);
		GH.windows().createAndPushWindow<CSpellWindow>(hero, owner.getCurrentPlayerInterface());
	}
}

void BattleHero::heroRightClicked()
{
	if(settings["battle"]["stickyHeroInfoWindows"].Bool())
		return;

	Point windowPosition;
	if(GH.screenDimensions().x < 1000)
	{
		windowPosition.x = (!defender) ? owner.fieldController->pos.left() + 1 : owner.fieldController->pos.right() - 79;
		windowPosition.y = owner.fieldController->pos.y + 135;
	}
	else
	{
		windowPosition.x = (!defender) ? owner.fieldController->pos.left() - 93 : owner.fieldController->pos.right() + 15;
		windowPosition.y = owner.fieldController->pos.y;
	}

	InfoAboutHero targetHero;
	if(owner.makingTurn() || settings["session"]["spectate"].Bool())
	{
		auto h = defender ? owner.defendingHeroInstance : owner.attackingHeroInstance;
		targetHero.initFromHero(h, InfoAboutHero::EInfoLevel::INBATTLE);
		GH.windows().createAndPushWindow<HeroInfoWindow>(targetHero, &windowPosition);
	}
}

void BattleHero::switchToNextPhase()
{
	phase = nextPhase;
	currentFrame = 0.f;

	auto copy = phaseFinishedCallback;
	phaseFinishedCallback.clear();
	copy();
}

BattleHero::BattleHero(const BattleInterface & owner, const CGHeroInstance * hero, bool defender):
	defender(defender),
	hero(hero),
	owner(owner),
	phase(EHeroAnimType::HOLDING),
	nextPhase(EHeroAnimType::HOLDING),
	currentSpeed(0.f),
	currentFrame(0.f),
	flagCurrentFrame(0.f)
{
	AnimationPath animationPath;

	if(!hero->type->battleImage.empty())
		animationPath = hero->type->battleImage;
	else
	if(hero->gender == EHeroGender::FEMALE)
		animationPath = hero->type->heroClass->imageBattleFemale;
	else
		animationPath = hero->type->heroClass->imageBattleMale;

	animation = GH.renderHandler().loadAnimation(animationPath);
	animation->preload();

	pos.w = 64;
	pos.h = 136;
	pos.x = owner.fieldController->pos.x + (defender ? (owner.fieldController->pos.w - pos.w) : 0);
	pos.y = owner.fieldController->pos.y;

	if(defender)
		animation->verticalFlip();

	if(defender)
		flagAnimation = GH.renderHandler().loadAnimation(AnimationPath::builtin("CMFLAGR"));
	else
		flagAnimation = GH.renderHandler().loadAnimation(AnimationPath::builtin("CMFLAGL"));

	flagAnimation->preload();
	flagAnimation->playerColored(hero->tempOwner);

	switchToNextPhase();
	play();

	addUsedEvents(TIME);
}

HeroInfoBasicPanel::HeroInfoBasicPanel(const InfoAboutHero & hero, Point * position, bool initializeBackground)
	: CIntObject(0)
{
	OBJECT_CONSTRUCTION_CAPTURING(255-DISPOSE);
	if (position != nullptr)
		moveTo(*position);

	if(initializeBackground)
	{
		background = std::make_shared<CPicture>(ImagePath::builtin("CHRPOP"));
		background->getSurface()->setBlitMode(EImageBlitMode::OPAQUE);
		background->colorize(hero.owner);
	}

	initializeData(hero);
}

void HeroInfoBasicPanel::initializeData(const InfoAboutHero & hero)
{
	OBJ_CONSTRUCTION_CAPTURING_ALL_NO_DISPOSE;
	auto attack = hero.details->primskills[0];
	auto defense = hero.details->primskills[1];
	auto power = hero.details->primskills[2];
	auto knowledge = hero.details->primskills[3];
	auto morale = hero.details->morale;
	auto luck = hero.details->luck;
	auto currentSpellPoints = hero.details->mana;
	auto maxSpellPoints = hero.details->manaLimit;

	icons.push_back(std::make_shared<CAnimImage>(AnimationPath::builtin("PortraitsLarge"), hero.getIconIndex(), 0, 10, 6));

	//primary stats
	labels.push_back(std::make_shared<CLabel>(9, 75, EFonts::FONT_TINY, ETextAlignment::TOPLEFT, Colors::WHITE, CGI->generaltexth->allTexts[380] + ":"));
	labels.push_back(std::make_shared<CLabel>(9, 87, EFonts::FONT_TINY, ETextAlignment::TOPLEFT, Colors::WHITE, CGI->generaltexth->allTexts[381] + ":"));
	labels.push_back(std::make_shared<CLabel>(9, 99, EFonts::FONT_TINY, ETextAlignment::TOPLEFT, Colors::WHITE, CGI->generaltexth->allTexts[382] + ":"));
	labels.push_back(std::make_shared<CLabel>(9, 111, EFonts::FONT_TINY, ETextAlignment::TOPLEFT, Colors::WHITE, CGI->generaltexth->allTexts[383] + ":"));

	labels.push_back(std::make_shared<CLabel>(69, 87, EFonts::FONT_TINY, ETextAlignment::BOTTOMRIGHT, Colors::WHITE, std::to_string(attack)));
	labels.push_back(std::make_shared<CLabel>(69, 99, EFonts::FONT_TINY, ETextAlignment::BOTTOMRIGHT, Colors::WHITE, std::to_string(defense)));
	labels.push_back(std::make_shared<CLabel>(69, 111, EFonts::FONT_TINY, ETextAlignment::BOTTOMRIGHT, Colors::WHITE, std::to_string(power)));
	labels.push_back(std::make_shared<CLabel>(69, 123, EFonts::FONT_TINY, ETextAlignment::BOTTOMRIGHT, Colors::WHITE, std::to_string(knowledge)));

	//morale+luck
	labels.push_back(std::make_shared<CLabel>(9, 131, EFonts::FONT_TINY, ETextAlignment::TOPLEFT, Colors::WHITE, CGI->generaltexth->allTexts[384] + ":"));
	labels.push_back(std::make_shared<CLabel>(9, 143, EFonts::FONT_TINY, ETextAlignment::TOPLEFT, Colors::WHITE, CGI->generaltexth->allTexts[385] + ":"));

	icons.push_back(std::make_shared<CAnimImage>(AnimationPath::builtin("IMRL22"), morale + 3, 0, 47, 131));
	icons.push_back(std::make_shared<CAnimImage>(AnimationPath::builtin("ILCK22"), luck + 3, 0, 47, 143));

	//spell points
	labels.push_back(std::make_shared<CLabel>(39, 174, EFonts::FONT_TINY, ETextAlignment::CENTER, Colors::WHITE, CGI->generaltexth->allTexts[387]));
	labels.push_back(std::make_shared<CLabel>(39, 186, EFonts::FONT_TINY, ETextAlignment::CENTER, Colors::WHITE, std::to_string(currentSpellPoints) + "/" + std::to_string(maxSpellPoints)));
}

void HeroInfoBasicPanel::update(const InfoAboutHero & updatedInfo)
{
	icons.clear();
	labels.clear();

	initializeData(updatedInfo);
}

void HeroInfoBasicPanel::show(Canvas & to)
{
	showAll(to);
	CIntObject::show(to);
}

HeroInfoWindow::HeroInfoWindow(const InfoAboutHero & hero, Point * position)
	: CWindowObject(RCLICK_POPUP | SHADOW_DISABLED, ImagePath::builtin("CHRPOP"))
{
	OBJECT_CONSTRUCTION_CAPTURING(255-DISPOSE);
	if (position != nullptr)
		moveTo(*position);

	background->colorize(hero.owner); //maybe add this functionality to base class?

	content = std::make_shared<HeroInfoBasicPanel>(hero, nullptr, false);
}

BattleResultWindow::BattleResultWindow(const BattleResult & br, CPlayerInterface & _owner, bool allowReplay)
	: owner(_owner), currentVideo(BattleResultVideo::NONE)
{
	OBJECT_CONSTRUCTION_CAPTURING(255-DISPOSE);

	background = std::make_shared<CPicture>(ImagePath::builtin("CPRESULT"));
	background->colorize(owner.playerID);
	pos = center(background->pos);

	exit = std::make_shared<CButton>(Point(384, 505), AnimationPath::builtin("iok6432.def"), std::make_pair("", ""), [&](){ bExitf();}, EShortcut::GLOBAL_ACCEPT);
	exit->setBorderColor(Colors::METALLIC_GOLD);
	
	if(allowReplay)
	{
		repeat = std::make_shared<CButton>(Point(24, 505), AnimationPath::builtin("icn6432.def"), std::make_pair("", ""), [&](){ bRepeatf();}, EShortcut::GLOBAL_CANCEL);
		repeat->setBorderColor(Colors::METALLIC_GOLD);
		labels.push_back(std::make_shared<CLabel>(232, 520, FONT_MEDIUM, ETextAlignment::CENTER, Colors::WHITE, CGI->generaltexth->translate("vcmi.battleResultsWindow.applyResultsLabel")));
	}

	if(br.winner == 0) //attacker won
	{
		labels.push_back(std::make_shared<CLabel>(59, 124, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE, CGI->generaltexth->allTexts[410]));
	}
	else
	{
		labels.push_back(std::make_shared<CLabel>(59, 124, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE, CGI->generaltexth->allTexts[411]));
	}

	if(br.winner == 1)
	{
		labels.push_back(std::make_shared<CLabel>(412, 124, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE, CGI->generaltexth->allTexts[410]));
	}
	else
	{
		labels.push_back(std::make_shared<CLabel>(408, 124, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE, CGI->generaltexth->allTexts[411]));
	}

	labels.push_back(std::make_shared<CLabel>(232, 302, FONT_BIG, ETextAlignment::CENTER, Colors::YELLOW,  CGI->generaltexth->allTexts[407]));
	labels.push_back(std::make_shared<CLabel>(232, 332, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE, CGI->generaltexth->allTexts[408]));
	labels.push_back(std::make_shared<CLabel>(232, 428, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE, CGI->generaltexth->allTexts[409]));

	std::string sideNames[2] = {"N/A", "N/A"};

	for(int i = 0; i < 2; i++)
	{
		auto heroInfo = owner.cb->getBattle(br.battleID)->battleGetHeroInfo(i);
		const int xs[] = {21, 392};

		if(heroInfo.portraitSource.isValid()) //attacking hero
		{
			icons.push_back(std::make_shared<CAnimImage>(AnimationPath::builtin("PortraitsLarge"), heroInfo.getIconIndex(), 0, xs[i], 38));
			sideNames[i] = heroInfo.name;
		}
		else
		{
			auto stacks = owner.cb->getBattle(br.battleID)->battleGetAllStacks();
			vstd::erase_if(stacks, [i](const CStack * stack) //erase stack of other side and not coming from garrison
			{
				return stack->unitSide() != i || !stack->base;
			});

			auto best = vstd::maxElementByFun(stacks, [](const CStack * stack)
			{
				return stack->unitType()->getAIValue();
			});

			if(best != stacks.end()) //should be always but to be safe...
			{
				icons.push_back(std::make_shared<CAnimImage>(AnimationPath::builtin("TWCRPORT"), (*best)->unitType()->getIconIndex(), 0, xs[i], 38));
				sideNames[i] = (*best)->unitType()->getNamePluralTranslated();
			}
		}
	}

	//printing attacker and defender's names
	labels.push_back(std::make_shared<CLabel>(89, 37, FONT_SMALL, ETextAlignment::TOPLEFT, Colors::WHITE, sideNames[0]));
	labels.push_back(std::make_shared<CLabel>(381, 53, FONT_SMALL, ETextAlignment::BOTTOMRIGHT, Colors::WHITE, sideNames[1]));

	//printing casualties
	for(int step = 0; step < 2; ++step)
	{
		if(br.casualties[step].size()==0)
		{
			labels.push_back(std::make_shared<CLabel>(235, 360 + 97 * step, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE, CGI->generaltexth->allTexts[523]));
		}
		else
		{
			int xPos = 235 - ((int)br.casualties[step].size()*32 + ((int)br.casualties[step].size() - 1)*10)/2; //increment by 42 with each picture
			int yPos = 344 + step * 97;
			for(auto & elem : br.casualties[step])
			{
				auto creature = CGI->creatures()->getByIndex(elem.first);
				if (creature->getId() == CreatureID::ARROW_TOWERS )
					continue; // do not show destroyed towers in battle results

				icons.push_back(std::make_shared<CAnimImage>(AnimationPath::builtin("CPRSMALL"), creature->getIconIndex(), 0, xPos, yPos));
				std::ostringstream amount;
				amount<<elem.second;
				labels.push_back(std::make_shared<CLabel>(xPos + 16, yPos + 42, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE, amount.str()));
				xPos += 42;
			}
		}
	}
	//printing result description
	bool weAreAttacker = !(owner.cb->getBattle(br.battleID)->battleGetMySide());
	if((br.winner == 0 && weAreAttacker) || (br.winner == 1 && !weAreAttacker)) //we've won
	{
		int text = 304;
		currentVideo = BattleResultVideo::WIN;
		switch(br.result)
		{
		case EBattleResult::NORMAL:
			if(owner.cb->getBattle(br.battleID)->battleGetDefendedTown() && !weAreAttacker)
				currentVideo = BattleResultVideo::WIN_SIEGE;
			break;
		case EBattleResult::ESCAPE:
			text = 303;
			break;
		case EBattleResult::SURRENDER:
			text = 302;
			break;
		default:
			logGlobal->error("Invalid battle result code %d. Assumed normal.", static_cast<int>(br.result));
			break;
		}
		playVideo();

		std::string str = CGI->generaltexth->allTexts[text];

		const CGHeroInstance * ourHero = owner.cb->getBattle(br.battleID)->battleGetMyHero();
		if (ourHero)
		{
			str += CGI->generaltexth->allTexts[305];
			boost::algorithm::replace_first(str, "%s", ourHero->getNameTranslated());
			boost::algorithm::replace_first(str, "%d", std::to_string(br.exp[weAreAttacker ? 0 : 1]));
		}

		description = std::make_shared<CTextBox>(str, Rect(69, 203, 330, 68), 0, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE);
	}
	else // we lose
	{
		int text = 311;
		currentVideo = BattleResultVideo::DEFEAT;
		switch(br.result)
		{
		case EBattleResult::NORMAL:
			if(owner.cb->getBattle(br.battleID)->battleGetDefendedTown() && !weAreAttacker)
				currentVideo = BattleResultVideo::DEFEAT_SIEGE;
			break;
		case EBattleResult::ESCAPE:
			currentVideo = BattleResultVideo::RETREAT;
			text = 310;
			break;
		case EBattleResult::SURRENDER:
			currentVideo = BattleResultVideo::SURRENDER;
			text = 309;
			break;
		default:
			logGlobal->error("Invalid battle result code %d. Assumed normal.", static_cast<int>(br.result));
			break;
		}
		playVideo();

		labels.push_back(std::make_shared<CLabel>(235, 235, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE, CGI->generaltexth->allTexts[text]));
	}
}

void BattleResultWindow::activate()
{
	owner.showingDialog->set(true);
	CIntObject::activate();
}

void BattleResultWindow::show(Canvas & to)
{
	CIntObject::show(to);
	CCS->videoh->update(pos.x + 107, pos.y + 70, to.getInternalSurface(), true, false,
	[&]()
	{
		playVideo(true);
	});
}

void BattleResultWindow::playVideo(bool startLoop)
{
	AudioPath musicName = AudioPath();
	VideoPath videoName = VideoPath();

	if(!startLoop)
	{
		switch(currentVideo)
		{
			case BattleResultVideo::WIN:
				musicName = AudioPath::builtin("Music/Win Battle");
				videoName = VideoPath::builtin("WIN3.BIK");
				break;
			case BattleResultVideo::SURRENDER:
				musicName = AudioPath::builtin("Music/Surrender Battle");
				videoName = VideoPath::builtin("SURRENDER.BIK");
				break;
			case BattleResultVideo::RETREAT:
				musicName = AudioPath::builtin("Music/Retreat Battle");
				videoName = VideoPath::builtin("RTSTART.BIK");
				break;
			case BattleResultVideo::DEFEAT:
				musicName = AudioPath::builtin("Music/LoseCombat");
				videoName = VideoPath::builtin("LBSTART.BIK");
				break;
			case BattleResultVideo::DEFEAT_SIEGE:
				musicName = AudioPath::builtin("Music/LoseCastle");
				videoName = VideoPath::builtin("LOSECSTL.BIK");	
				break;
			case BattleResultVideo::WIN_SIEGE:
				musicName = AudioPath::builtin("Music/Defend Castle");
				videoName = VideoPath::builtin("DEFENDALL.BIK");	
				break;
		}
	}
	else
	{
		switch(currentVideo)
		{
			case BattleResultVideo::RETREAT:
				currentVideo = BattleResultVideo::RETREAT_LOOP;
				videoName = VideoPath::builtin("RTLOOP.BIK");
				break;
			case BattleResultVideo::DEFEAT:
				currentVideo = BattleResultVideo::DEFEAT_LOOP;
				videoName = VideoPath::builtin("LBLOOP.BIK");
				break;
			case BattleResultVideo::DEFEAT_SIEGE:
				currentVideo = BattleResultVideo::DEFEAT_SIEGE_LOOP;
				videoName = VideoPath::builtin("LOSECSLP.BIK");	
				break;
			case BattleResultVideo::WIN_SIEGE:
				currentVideo = BattleResultVideo::WIN_SIEGE_LOOP;
				videoName = VideoPath::builtin("DEFENDLOOP.BIK");	
				break;
		}	
	}

	if(musicName != AudioPath())
		CCS->musich->playMusic(musicName, false, true);
	
	if(videoName != VideoPath())
		CCS->videoh->open(videoName);
}

void BattleResultWindow::buttonPressed(int button)
{
	if (resultCallback)
		resultCallback(button);

	CPlayerInterface &intTmp = owner; //copy reference because "this" will be destructed soon

	close();

	if(GH.windows().topWindow<BattleWindow>())
		GH.windows().popWindows(1); //pop battle interface if present

	//Result window and battle interface are gone. We requested all dialogs to be closed before opening the battle,
	//so we can be sure that there is no dialogs left on GUI stack.
	intTmp.showingDialog->setn(false);
	CCS->videoh->close();
}

void BattleResultWindow::bExitf()
{
	buttonPressed(0);
}

void BattleResultWindow::bRepeatf()
{
	buttonPressed(1);
}

StackQueue::StackQueue(bool Embedded, BattleInterface & owner)
	: embedded(Embedded),
	owner(owner)
{
	OBJECT_CONSTRUCTION_CAPTURING(255-DISPOSE);
	if(embedded)
	{
		pos.w = QUEUE_SIZE * 41;
		pos.h = 49;
		pos.x += parent->pos.w/2 - pos.w/2;
		pos.y += 10;

		icons = GH.renderHandler().loadAnimation(AnimationPath::builtin("CPRSMALL"));
		stateIcons = GH.renderHandler().loadAnimation(AnimationPath::builtin("VCMI/BATTLEQUEUE/STATESSMALL"));
	}
	else
	{
		pos.w = 800;
		pos.h = 85;
		pos.x += 0;
		pos.y -= pos.h;

		background = std::make_shared<CFilledTexture>(ImagePath::builtin("DIBOXBCK"), Rect(0, 0, pos.w, pos.h));

		icons = GH.renderHandler().loadAnimation(AnimationPath::builtin("TWCRPORT"));
		stateIcons = GH.renderHandler().loadAnimation(AnimationPath::builtin("VCMI/BATTLEQUEUE/STATESSMALL"));
		//TODO: where use big icons?
		//stateIcons = GH.renderHandler().loadAnimation("VCMI/BATTLEQUEUE/STATESBIG");
	}
	stateIcons->preload();

	stackBoxes.resize(QUEUE_SIZE);
	for (int i = 0; i < stackBoxes.size(); i++)
	{
		stackBoxes[i] = std::make_shared<StackBox>(this);
		stackBoxes[i]->moveBy(Point(1 + (embedded ? 41 : 80) * i, 0));
	}
}

void StackQueue::show(Canvas & to)
{
	if (embedded)
		showAll(to);
	CIntObject::show(to);
}

void StackQueue::update()
{
	std::vector<battle::Units> queueData;

	owner.getBattle()->battleGetTurnOrder(queueData, stackBoxes.size(), 0);

	size_t boxIndex = 0;

	for(size_t turn = 0; turn < queueData.size() && boxIndex < stackBoxes.size(); turn++)
	{
		for(size_t unitIndex = 0; unitIndex < queueData[turn].size() && boxIndex < stackBoxes.size(); boxIndex++, unitIndex++)
			stackBoxes[boxIndex]->setUnit(queueData[turn][unitIndex], turn);
	}

	for(; boxIndex < stackBoxes.size(); boxIndex++)
		stackBoxes[boxIndex]->setUnit(nullptr);
}

int32_t StackQueue::getSiegeShooterIconID()
{
	return owner.siegeController->getSiegedTown()->town->faction->getIndex();
}

std::optional<uint32_t> StackQueue::getHoveredUnitIdIfAny() const
{
	for(const auto & stackBox : stackBoxes)
	{
		if(stackBox->isHovered())
		{
			return stackBox->getBoundUnitID();
		}
	}

	return std::nullopt;
}

StackQueue::StackBox::StackBox(StackQueue * owner):
	CIntObject(SHOW_POPUP | HOVER), owner(owner)
{
	OBJECT_CONSTRUCTION_CAPTURING(255-DISPOSE);
	background = std::make_shared<CPicture>(ImagePath::builtin(owner->embedded ? "StackQueueSmall" : "StackQueueLarge"));

	pos.w = background->pos.w;
	pos.h = background->pos.h;

	if(owner->embedded)
	{
		icon = std::make_shared<CAnimImage>(owner->icons, 0, 0, 5, 2);
		amount = std::make_shared<CLabel>(pos.w/2, pos.h - 7, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE);
	}
	else
	{
		icon = std::make_shared<CAnimImage>(owner->icons, 0, 0, 9, 1);
		amount = std::make_shared<CLabel>(pos.w/2, pos.h - 8, FONT_MEDIUM, ETextAlignment::CENTER, Colors::WHITE);

		int icon_x = pos.w - 17;
		int icon_y = pos.h - 18;

		stateIcon = std::make_shared<CAnimImage>(owner->stateIcons, 0, 0, icon_x, icon_y);
		stateIcon->visible = false;
	}
}

void StackQueue::StackBox::setUnit(const battle::Unit * unit, size_t turn)
{
	if(unit)
	{
		boundUnitID = unit->unitId();
		background->colorize(unit->unitOwner());
		icon->visible = true;

		// temporary code for mod compatibility:
		// first, set icon that should definitely exist (arrow tower icon in base vcmi mod)
		// second, try to switch to icon that should be provided by mod
		// if mod is not up to date and does have arrow tower icon yet - second setFrame call will fail and retain previously set image
		// for 1.2 release & later next line should be moved into 'else' block
		icon->setFrame(unit->creatureIconIndex(), 0);
		if (unit->unitType()->getId() == CreatureID::ARROW_TOWERS)
			icon->setFrame(owner->getSiegeShooterIconID(), 1);

		amount->setText(TextOperations::formatMetric(unit->getCount(), 4));

		if(stateIcon)
		{
			if(unit->defended((int)turn) || (turn > 0 && unit->defended((int)turn - 1)))
			{
				stateIcon->setFrame(0, 0);
				stateIcon->visible = true;
			}
			else if(unit->waited((int)turn))
			{
				stateIcon->setFrame(1, 0);
				stateIcon->visible = true;
			}
			else
			{
				stateIcon->visible = false;
			}
		}
	}
	else
	{
		boundUnitID = std::nullopt;
		background->colorize(PlayerColor::NEUTRAL);
		icon->visible = false;
		icon->setFrame(0);
		amount->setText("");

		if(stateIcon)
			stateIcon->visible = false;
	}
}

std::optional<uint32_t> StackQueue::StackBox::getBoundUnitID() const
{
	return boundUnitID;
}

bool StackQueue::StackBox::isBoundUnitHighlighted() const
{
	auto unitIdsToHighlight = owner->owner.stacksController->getHoveredStacksUnitIds();
	return vstd::contains(unitIdsToHighlight, getBoundUnitID());
}

void StackQueue::StackBox::showAll(Canvas & to)
{
	CIntObject::showAll(to);

	if(isBoundUnitHighlighted())
		to.drawBorder(background->pos, Colors::CYAN, 2);
}

void StackQueue::StackBox::show(Canvas & to)
{
	CIntObject::show(to);

	if(isBoundUnitHighlighted())
		to.drawBorder(background->pos, Colors::CYAN, 2);
}
