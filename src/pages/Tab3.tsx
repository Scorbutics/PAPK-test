import { IonContent, IonHeader, IonPage, IonTitle, IonToolbar } from '@ionic/react';
import ExploreContainer from '../components/ExploreContainer';
import './Tab3.css';
import { Input, InputWithSeparatorContainer, Label } from 'pokemon-studio-components';


const Tab3: React.FC = () => {
  console.log('Tab3 loaded');
  return (
    <IonPage>
      <IonHeader>
        <IonToolbar>
          <IonTitle>Tab 3</IonTitle>
        </IonToolbar>
      </IonHeader>
      <IonContent fullscreen>
        <IonHeader collapse="condense">
          <IonToolbar>
            <IonTitle size="large">Tab 3</IonTitle>
          </IonToolbar>
        </IonHeader>

      <InputWithSeparatorContainer>
        <Label>Coucou</Label>
        <Input name="projectPath" />
      </InputWithSeparatorContainer>
      
      </IonContent>
    </IonPage>
  );
};

export default Tab3;
